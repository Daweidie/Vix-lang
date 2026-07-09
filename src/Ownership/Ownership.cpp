/**
 * @file ownership.cpp
 * @brief Ownership and borrow checker implementation
 *
 * This module traverses the abstract syntax tree (AST) and enforces Rust-like
 * ownership and borrowing rules. Core features include:
 *   - Tracking the move state of each variable
 *   - Checking the legality of mutable/immutable borrows (only one mutable
 * borrow or any number of immutable borrows at a time)
 *   - Handling control-flow branch (if/else) state merging
 *   - Supporting transaction rollback for branch preview
 *   - Checking whether a returned borrow refers to a local variable
 */

#include "../../include/ownership.h"
#include "../../include/compiler.h"

#include <cctype>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

extern "C" const char *current_input_filename;

namespace {

/**
 * @brief Expression usage type, used to distinguish read, move, take address,
 * and assignment target
 */
enum class ExprUse { Read, Move, Address, AssignTarget };

/**
 * @brief Full state of a variable, used to track ownership, borrowing, and
 * moves
 */
struct VarState {
  std::string name;               ///< Variable name
  int scope_level = -1;           ///< Scope depth (0 is global)
  const TypeInfo *type = nullptr; ///< Type information
  bool is_mutable = false;        ///< Whether mutable
  bool moved = false;             ///< Whether moved (ownership transferred)
  ASTNode *moved_at =
      nullptr; ///< AST node where the move occurred (for error reporting)
  bool is_global = false; ///< Whether a global variable
  bool borrowed_shared =
      false;                 ///< Whether there is a shared (immutable) borrow
  bool borrowed_mut = false; ///< Whether there is an exclusive (mutable) borrow
  std::unordered_set<std::string>
      borrow_sources; ///< Source variable names of current borrows (for chain
                      ///< tracking)
};

/**
 * @brief A scope containing all variables within that scope
 */
struct Scope {
  std::unordered_map<std::string, VarState> vars;
};

/**
 * @brief Information after evaluating an expression
 */
struct ExprInfo {
  const TypeInfo *type = nullptr; ///< Expression type
  bool copy = true;               ///< Whether copyable (Copy trait)
  bool is_ref = false;            ///< Whether a reference type
  std::unordered_set<std::string>
      borrow_sources; ///< Source variables that this expression refers to
};

/**
 * @brief Transaction log entry, recording the state of a variable before the
 * transaction began
 */
struct LogEntry {
  int scope_level;
  std::string name;
  VarState old_state;
};

/**
 * @brief Branch state snapshot, used for if/else merging
 */
struct BranchState {
  bool moved = false;
  bool borrowed_shared = false;
  bool borrowed_mut = false;
  std::unordered_set<std::string> borrow_sources;
};

/**
 * @brief Main ownership checker class
 *
 * Traverses the AST, maintains a scope stack and variable states, and detects
 * errors such as use-after-move and illegal borrows. Supports a transaction
 * mechanism to handle branch merging: record state before entering a branch,
 * roll back after branch checking, then merge the side effects of each branch.
 */
class OwnershipChecker {
public:
  /**
   * @brief Public entry point: perform ownership checking on the entire program
   * AST
   * @param root AST root node
   * @return 0 indicates check passed, 1 indicates errors exist
   */
  int check(ASTNode *root) {
    push_scope();
    check_node(root);
    pop_scope();
    return errors;
  }

private:
  std::vector<Scope> scopes; ///< Scope stack
  int errors = 0;            ///< Accumulated error count

  // Transaction related
  std::vector<std::vector<LogEntry>>
      transaction_logs; ///< Transaction log stack (one log list per
                        ///< transaction)
  size_t transaction_baseline_depth =
      0; ///< Scope depth at transaction start, used to limit logging range

  static const char *node_file(const ASTNode *node) {
    if (node && node->source_file)
      return node->source_file;
    return current_input_filename ? current_input_filename : "unknown";
  }

  static int node_line(const ASTNode *node) {
    return node && node->location.first_line > 0 ? node->location.first_line
                                                 : 1;
  }

  static int node_col(const ASTNode *node) {
    return node && node->location.first_column > 0 ? node->location.first_column
                                                   : 1;
  }

  /**
   * @brief Determine whether a type satisfies Copy semantics (can be implicitly
   * copied)
   * @details Primitives, pointers, function pointers are always Copy;
   *          structs are Copy if all fields are Copy;
   *          strings, dynamic arrays, generic applications are non-Copy by
   * default; fixed-size arrays are considered Copy if size ≤ 16 and element
   * type is Copy (optimization for small arrays)
   */
  static bool is_copy_type(const TypeInfo *t) {
    if (!t)
      return true;
    switch (t->kind) {
    case TYPEINFO_VOID:
    case TYPEINFO_I8:
    case TYPEINFO_I32:
    case TYPEINFO_I64:
    case TYPEINFO_F32:
    case TYPEINFO_F64:
    case TYPEINFO_BOOL:
    case TYPEINFO_PTR:
    case TYPEINFO_FN:
    case TYPEINFO_VAR:
      return true;
    case TYPEINFO_STRING:
      return false;
    case TYPEINFO_STRUCT:
      if (t->params && t->param_count > 0) {
        for (int i = 0; i < t->param_count; i++) {
          if (!is_copy_type(t->params[i]))
            return false;
        }
      }
      return true;
    case TYPEINFO_FIXED_ARRAY:
      return t->size <= 16 && is_copy_type(t->element);
    case TYPEINFO_ARRAY:
    case TYPEINFO_APP:
      return false;
    }
    return true;
  }

  void push_scope() { scopes.emplace_back(); }
  void pop_scope() {
    if (!scopes.empty())
      scopes.pop_back();
  }

  /**
   * @brief Report a semantic error, set location and call the frontend error
   * interface
   */
  void report(ASTNode *node, const std::string &message) {
    set_location_with_column(node_file(node), node_line(node), node_col(node));
    int length = 1;
    // Extract variable name from messages like "use of moved value 'buf'"
    auto start = message.find('\'');
    auto end = message.find('\'', start + 1);
    if (start != std::string::npos && end != std::string::npos && end > start) {
      std::string vname = message.substr(start + 1, end - start - 1);
      adjust_column_to_identifier(vname.c_str());
      length = (int)vname.size();
    }
    report_simple_error_with_length(ERROR_LEVEL_ERROR, ERROR_SEMANTIC,
                                    message.c_str(), length);
    errors++;
  }

  /**
   * @brief Look up a variable starting from the current scope upwards
   */
  VarState *lookup(const std::string &name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto found = it->vars.find(name);
      if (found != it->vars.end())
        return &found->second;
    }
    return nullptr;
  }

  /**
   * @brief Check whether a name is a local variable (non-global)
   */
  bool is_local_name(const std::string &name) const {
    if (name.empty())
      return false;
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto found = it->vars.find(name);
      if (found != it->vars.end())
        return !found->second.is_global;
    }
    return false;
  }

  /**
   * @brief Begin a new transaction for branch preview
   * @details Record the current scope depth; subsequent modifications to
   * variables will be logged, so they can be restored on rollback.
   */
  void begin_transaction() {
    transaction_logs.emplace_back();
    transaction_baseline_depth = scopes.size();
  }

  /**
   * @brief Commit the current transaction, merging the log into the parent
   * transaction (if any)
   * @details If a parent transaction exists, merge current log entries into the
   * parent log (deduplicating), indicating that these modifications should be
   * preserved after branch merging.
   */
  void commit_transaction() {
    if (transaction_logs.empty())
      return;
    auto committed = std::move(transaction_logs.back());
    transaction_logs.pop_back();
    if (!transaction_logs.empty()) {
      auto &parent = transaction_logs.back();
      for (auto &entry : committed) {
        bool present = false;
        for (auto &p : parent) {
          if (p.scope_level == entry.scope_level && p.name == entry.name) {
            present = true;
            break;
          }
        }
        if (!present)
          parent.push_back(std::move(entry));
      }
    }
  }

  /**
   * @brief Rollback the current transaction, restoring all modified variables
   * to their state before the transaction began
   */
  void rollback_transaction() {
    if (transaction_logs.empty())
      return;
    auto &log = transaction_logs.back();
    for (auto it = log.rbegin(); it != log.rend(); ++it) {
      if (it->scope_level >= 0 && it->scope_level < (int)scopes.size()) {
        auto &scope = scopes[it->scope_level];
        auto var = scope.vars.find(it->name);
        if (var != scope.vars.end()) {
          var->second = it->old_state;
        }
      }
    }
    transaction_logs.pop_back();
  }

  /**
   * @brief Log a variable modification for the transaction log
   * @details Only when currently in a transaction, the variable is in an outer
   * scope (not newly created within the transaction), and not yet recorded,
   * save the current state as the old state.
   */
  void log_var_modification(VarState *state) {
    if (transaction_logs.empty())
      return;
    if (state->scope_level < 0)
      return;
    if ((size_t)state->scope_level >= transaction_baseline_depth)
      return;
    auto &current_log = transaction_logs.back();
    for (auto &entry : current_log) {
      if (entry.scope_level == state->scope_level && entry.name == state->name)
        return;
    }
    current_log.push_back({state->scope_level, state->name, *state});
  }

  /**
   * @brief Capture the state of all currently visible variables (for if
   * baseline)
   */
  void
  capture_all_variables(std::unordered_map<std::string, BranchState> &out) {
    for (auto &scope : scopes) {
      for (auto &entry : scope.vars) {
        auto &var = entry.second;
        BranchState bs;
        bs.moved = var.moved;
        bs.borrowed_shared = var.borrowed_shared;
        bs.borrowed_mut = var.borrowed_mut;
        bs.borrow_sources = var.borrow_sources;
        out[var.name] = std::move(bs);
      }
    }
  }

  /**
   * @brief Capture variable states in outer scopes (outside the transaction
   * baseline), used for post-branch snapshots
   */
  void capture_outer_branch_state(
      std::unordered_map<std::string, BranchState> &out) {
    for (auto &scope : scopes) {
      for (auto &entry : scope.vars) {
        auto &var = entry.second;
        if (var.scope_level < 0 ||
            (size_t)var.scope_level >= transaction_baseline_depth)
          continue;
        BranchState bs;
        bs.moved = var.moved;
        bs.borrowed_shared = var.borrowed_shared;
        bs.borrowed_mut = var.borrowed_mut;
        bs.borrow_sources = var.borrow_sources;
        out[var.name] = std::move(bs);
      }
    }
  }

  void declare_var(const std::string &name, const TypeInfo *type,
                   bool is_mutable, bool is_global,
                   const std::unordered_set<std::string> &borrow_sources = {}) {
    if (scopes.empty())
      push_scope();
    VarState state;
    state.name = name;
    state.scope_level = (int)scopes.size() - 1;
    state.type = type;
    state.is_mutable = is_mutable;
    state.is_global = is_global;
    state.borrow_sources = borrow_sources;
    scopes.back().vars[name] = std::move(state);
  }

  /**
   * @brief Cleanup work at the end of a statement
   * @details Clear borrow flags of variables that are no longer referenced by
   * any borrow. For example, if a borrow source variable is no longer in any
   * borrow_sources, then that variable's borrowed_shared/borrowed_mut should be
   * reset.
   */
  void end_statement() {
    // Collect all currently active borrow sources (i.e., names appearing in any
    // variable's borrow_sources)
    std::unordered_set<std::string> active_borrow_sources;
    for (auto &scope : scopes) {
      for (auto &entry : scope.vars) {
        for (const auto &src : entry.second.borrow_sources) {
          active_borrow_sources.insert(src);
        }
      }
    }
    // Iterate all variables; if a variable itself is not in the active borrow
    // sources, clear its borrow flags
    for (auto &scope : scopes) {
      for (auto &entry : scope.vars) {
        if (active_borrow_sources.find(entry.first) ==
            active_borrow_sources.end()) {
          log_var_modification(&entry.second);
          entry.second.borrowed_shared = false;
          entry.second.borrowed_mut = false;
        }
      }
    }
  }

  void check_stmt(ASTNode *stmt);

  void check_block(ASTNode *block, bool new_scope);

  void check_function(ASTNode *fn);

  void check_global(ASTNode *node);

  void check_return(ASTNode *node);

  void check_assign(ASTNode *node);

  ExprInfo check_expr(ASTNode *node, ExprUse use);

  ExprInfo check_identifier(ASTNode *node, ExprUse use);

  ExprInfo check_unary(ASTNode *node);

  ExprInfo check_call(ASTNode *node);

  ExprInfo check_expression_list(ASTNode *node, ExprUse use = ExprUse::Move);

  ExprInfo check_struct_literal(ASTNode *node);

  /**
   * @brief Count the number of dereference levels of an expression
   */
  static int count_derefs(ASTNode *node);

  /**
   * @brief Expand a variable name through pointer borrow_sources.
   *
   * @param name The variable name to start from.
   * @param max_levels Number of pointer dereferences to expand; -1 means
   * unlimited (until fixed point).
   * @return Set of all ultimate variable names after `max_levels` dereferences.
   */
  std::unordered_set<std::string>
  resolve_pointer_targets(const std::string &name, int max_levels = -1);

  /**
   * @brief Get the underlying variable name of an expression (stripping member
   * access, subscript, dereference etc.)
   * @return the bottommost identifier name, or empty if cannot be extracted
   */
  std::string lvalue_base(ASTNode *node);

  void check_node(ASTNode *node);
};

/**
 * @brief Expand a variable name through pointer borrow_sources.
 *
 * Starting from `name`, follow `borrow_sources` chains up to `max_levels`
 * times. If `max_levels` is -1, expand until no further pointer indirection
 * with known borrow sources is found. Cycles are prevented via a visited set.
 *
 * @return The set of all possible final variable names after the expansion.
 */
std::unordered_set<std::string>
OwnershipChecker::resolve_pointer_targets(const std::string &name,
                                          int max_levels) {
  std::unordered_set<std::string> current;
  if (name.empty())
    return {};
  current.insert(name);
  int level = 0;
  while ((max_levels == -1 || level < max_levels) && !current.empty()) {
    std::unordered_set<std::string> next;
    for (const auto &var : current) {
      VarState *state = lookup(var);
      if (!state) {
        next.insert(var);
        continue;
      }
      if (state->type && state->type->kind == TYPEINFO_PTR &&
          !state->borrow_sources.empty()) {
        next.insert(state->borrow_sources.begin(), state->borrow_sources.end());
      } else {
        next.insert(var);
      }
    }
    if (next == current)
      break;
    current = std::move(next);
    ++level;
  }
  return current;
}

/**
 * @brief Check a single statement
 */
void OwnershipChecker::check_stmt(ASTNode *stmt) {
  if (!stmt)
    return;
  switch (stmt->type) {
  case AST_PROGRAM:
    check_block(stmt, true);
    return;
  case AST_FUNCTION:
    check_function(stmt);
    return;
  case AST_ASSIGN:
  case AST_CONST:
    check_assign(stmt);
    end_statement();
    return;
  case AST_PRINT:
    check_expr(stmt->data.print.expr, ExprUse::Read);
    end_statement();
    return;
  case AST_RETURN:
    check_return(stmt);
    end_statement();
    return;
  case AST_IF: {
    // Check condition expression (read-only)
    check_expr(stmt->data.if_stmt.condition, ExprUse::Read);
    end_statement();

    bool has_else = (stmt->data.if_stmt.else_body != nullptr);

    // Save state before entering the branch as baseline
    std::unordered_map<std::string, BranchState> baseline;
    capture_all_variables(baseline);

    // Check then branch, use transaction for rollback
    begin_transaction();
    check_block(stmt->data.if_stmt.then_body, true);
    std::unordered_map<std::string, BranchState> then_state;
    capture_outer_branch_state(then_state);
    rollback_transaction();

    // Check else branch (if any), also use transaction
    std::unordered_map<std::string, BranchState> else_state;
    if (has_else) {
      begin_transaction();
      check_block(stmt->data.if_stmt.else_body, true);
      capture_outer_branch_state(else_state);
      rollback_transaction();
    }

    // Collect all variables that appear in then/else
    std::unordered_set<std::string> all_keys;
    for (auto &p : then_state)
      all_keys.insert(p.first);
    if (has_else)
      for (auto &p : else_state)
        all_keys.insert(p.first);

    // Merge branch states: use "or" logic (if moved or borrowed in any branch,
    // keep after merge)
    for (const auto &var_name : all_keys) {
      auto base_it = baseline.find(var_name);
      BranchState base_bs;
      if (base_it != baseline.end())
        base_bs = base_it->second;

      BranchState then_bs = base_bs;
      auto then_it = then_state.find(var_name);
      if (then_it != then_state.end())
        then_bs = then_it->second;

      BranchState else_bs = base_bs;
      if (has_else) {
        auto else_it = else_state.find(var_name);
        if (else_it != else_state.end())
          else_bs = else_it->second;
      }

      bool merged_moved =
          has_else ? (then_bs.moved || else_bs.moved) : then_bs.moved;
      bool merged_shared =
          has_else ? (then_bs.borrowed_shared || else_bs.borrowed_shared)
                   : then_bs.borrowed_shared;
      bool merged_mut = has_else
                            ? (then_bs.borrowed_mut || else_bs.borrowed_mut)
                            : then_bs.borrowed_mut;

      auto merged_sources = then_bs.borrow_sources;
      if (has_else)
        merged_sources.insert(else_bs.borrow_sources.begin(),
                              else_bs.borrow_sources.end());

      VarState *state = lookup(var_name);
      if (state) {
        state->moved = merged_moved;
        state->borrowed_shared = merged_shared;
        state->borrowed_mut = merged_mut;
        state->borrow_sources = std::move(merged_sources);
        // If variable was moved and is non-Copy, clear borrow sources (can no
        // longer be borrowed)
        if (state->moved && !is_copy_type(state->type)) {
          state->borrow_sources.clear();
        }
      }
    }
    return;
  }
  case AST_WHILE:
    check_expr(stmt->data.while_stmt.condition, ExprUse::Read);
    end_statement();
    check_block(stmt->data.while_stmt.body, true);
    return;
  case AST_FOR:
    check_expr(stmt->data.for_stmt.start, ExprUse::Read);
    check_expr(stmt->data.for_stmt.end, ExprUse::Read);
    end_statement();
    {
      push_scope();
      if (stmt->data.for_stmt.var &&
          stmt->data.for_stmt.var->type == AST_IDENTIFIER &&
          stmt->data.for_stmt.var->data.identifier.name) {
        declare_var(stmt->data.for_stmt.var->data.identifier.name,
                    stmt->data.for_stmt.var->inferred_type, false, false);
      }
      check_block(stmt->data.for_stmt.body, false);
      pop_scope();
      end_statement();
    }
    return;
  case AST_BREAK:
  case AST_CONTINUE:
    end_statement();
    return;
  default:
    check_expr(stmt, ExprUse::Read);
    end_statement();
    return;
  }
}

/**
 * @brief Check a code block
 * @param block code block node (could be AST_PROGRAM or a single statement)
 * @param new_scope whether it is a new scope
 */
void OwnershipChecker::check_block(ASTNode *block, bool new_scope) {
  if (!block)
    return;
  if (new_scope)
    push_scope();

  if (block->type == AST_PROGRAM) {
    for (int i = 0; i < block->data.program.statement_count; i++) {
      check_stmt(block->data.program.statements[i]);
    }
  } else {
    check_stmt(block);
  }

  if (new_scope) {
    pop_scope();
    end_statement();
  }
}

/**
 * @brief Check a function definition
 * @details Process parameter declarations and check the function body
 */
void OwnershipChecker::check_function(ASTNode *fn) {
  if (!fn || fn->data.function.is_extern)
    return;
  push_scope();
  ASTNode *params = fn->data.function.params;
  if (params && params->type == AST_EXPRESSION_LIST) {
    for (int i = 0; i < params->data.expression_list.expression_count; i++) {
      ASTNode *param = params->data.expression_list.expressions[i];
      if (!param)
        continue;
      ASTNode *id = param;
      const TypeInfo *type = param->inferred_type;
      bool mut = param->mutability == MUTABILITY_MUTABLE;
      if (param->type == AST_ASSIGN) { // Parameter with default value
        id = param->data.assign.left;
        type = param->inferred_type
                   ? param->inferred_type
                   : (param->data.assign.right
                          ? param->data.assign.right->inferred_type
                          : nullptr);
        mut = param->data.assign.mutability == MUTABILITY_MUTABLE ||
              (id && id->mutability == MUTABILITY_MUTABLE);
        if (param->data.assign.right) {
          push_scope();
          check_expr(param->data.assign.right, ExprUse::Read);
          pop_scope();
          end_statement();
        }
      }
      if (id && id->type == AST_IDENTIFIER && id->data.identifier.name) {
        declare_var(id->data.identifier.name, type, mut, false);
      }
    }
  }
  check_block(fn->data.function.body, false);
  pop_scope();
  end_statement();
}

/**
 * @brief Check a global variable declaration
 */
void OwnershipChecker::check_global(ASTNode *node) {
  if (!node || !node->data.global_decl.identifier ||
      node->data.global_decl.identifier->type != AST_IDENTIFIER) {
    return;
  }
  ExprInfo init = check_expr(node->data.global_decl.initializer, ExprUse::Read);
  const char *name = node->data.global_decl.identifier->data.identifier.name;
  if (name)
    declare_var(name, node->inferred_type ? node->inferred_type : init.type,
                true, true, init.borrow_sources);
}

/**
 * @brief Check a return statement
 * @details Ensure the return value does not reference local variables (prevent
 * dangling references)
 */
void OwnershipChecker::check_return(ASTNode *node) {
  ExprInfo value = check_expr(node->data.return_stmt.expr, ExprUse::Move);
  for (const auto &src : value.borrow_sources) {
    if (is_local_name(src)) {
      report(node->data.return_stmt.expr ? node->data.return_stmt.expr : node,
             "cannot return reference to local variable '" + src + "'");
      break;
    }
  }
}

/**
 * @brief Check an assignment statement (including declarations)
 * @details Handle variable declarations, move semantics, and borrow checking.
 *          **Fixed**:
 *           - Dereference assignments (*ptr = ...) no longer mutate the
 * pointer's ownership state (moved flag, borrow_sources). The pointer is only
 *             checked for liveness and borrow flags.
 *           - Reading or otherwise using a mutably borrowed variable is now
 * rejected.
 *           - Writing through a borrowed pointer is rejected.
 */
void OwnershipChecker::check_assign(ASTNode *node) {
  ASTNode *left = node->data.assign.left;
  ASTNode *right = node->data.assign.right;
  bool is_decl = node->data.assign.is_declaration != 0;
  ExprInfo rhs = check_expr(right, ExprUse::Move);

  // Variable declaration
  if (is_decl && left && left->type == AST_IDENTIFIER &&
      left->data.identifier.name) {
    if (!rhs.borrow_sources.empty() &&
        rhs.borrow_sources.count(left->data.identifier.name)) {
      report(left, "cannot borrow value of variable being declared");
    }
    bool mut = left->mutability == MUTABILITY_MUTABLE ||
               node->data.assign.mutability == MUTABILITY_MUTABLE;
    const TypeInfo *type = right ? right->inferred_type : nullptr;
    if (!type || type->kind == TYPEINFO_VOID)
      type = rhs.type;
    declare_var(left->data.identifier.name, type ? type : rhs.type, mut,
                scopes.size() == 1, rhs.borrow_sources);
    return;
  }

  // Assignment to existing variable (or through a pointer)
  std::string base = lvalue_base(left);
  if (!base.empty()) {
    int deref_count = count_derefs(left);
    auto target_set = resolve_pointer_targets(base, deref_count);

    // Ambiguous assignment target (multiple possible lvalues) – always reject.
    if (target_set.size() > 1) {
      report(left,
             "cannot assign through pointer with multiple possible targets");
      return;
    }

    std::string target;
    if (!target_set.empty()) {
      target = *target_set.begin();
    } else {
      target = base;
    }

    VarState *target_state = lookup(target);
    if (target_state) {
      log_var_modification(target_state);

      // --- Distinguish direct assignment from indirect (dereference)
      // assignment ---
      bool is_indirect = (deref_count > 0);

      if (is_indirect) {
        // Indirect assignment: *ptr = expr, etc.
        // The pointer itself is only read; its ownership must NOT change, but
        // we must ensure it is still alive and not borrowed.
        if (target_state->moved) {
          report(left, "use of moved value '" + target + "'");
          return;
        }
        if (target_state->borrowed_shared || target_state->borrowed_mut) {
          report(left,
                 "cannot assign through '" + target + "' while it is borrowed");
          return;
        }
        // No state mutation for the pointer.
      } else {
        // Direct assignment to a variable or its field (e.g. x = ..., x.field =
        // ...)

        // Borrow conflict: cannot assign to a variable while it is borrowed
        if (target_state->borrowed_shared || target_state->borrowed_mut) {
          report(left,
                 "cannot assign to '" + target + "' while it is borrowed");
        }

        // Determine if partial assignment (field / element of a struct/array)
        bool is_partial = false;
        ASTNode *lv = left;
        while (lv && lv->type == AST_UNARYOP &&
               lv->data.unaryop.op == OP_DEREF) {
          lv = lv->data.unaryop.expr;
        }
        if (lv && (lv->type == AST_MEMBER_ACCESS || lv->type == AST_INDEX)) {
          is_partial = true;
        }

        // Handle moved state: full rebinding allowed, partial on moved is error
        if (target_state->moved) {
          if (is_partial) {
            report(left, "use of moved value '" + target + "'");
            return; // Do not modify state
          }
          // else: full rebinding of a moved variable is allowed
          // (reinitialization)
        }

        // Update ownership: rebinding clears moved, replaces borrow_sources
        target_state->moved = false;
        if (is_partial) {
          target_state->borrow_sources.insert(rhs.borrow_sources.begin(),
                                              rhs.borrow_sources.end());
        } else {
          target_state->borrow_sources = rhs.borrow_sources;
        }
      }
    }
  }

  // Validate the left-hand side expression path
  check_expr(left, ExprUse::AssignTarget);
}

/**
 * @brief Check an expression, returning expression information
 */
ExprInfo OwnershipChecker::check_expr(ASTNode *node, ExprUse use) {
  ExprInfo info;
  if (!node)
    return info;
  info.type = node->inferred_type;
  info.copy = is_copy_type(node->inferred_type);

  switch (node->type) {
  case AST_IDENTIFIER:
    return check_identifier(node, use);
  case AST_NUM_INT:
  case AST_NUM_FLOAT:
  case AST_CHAR:
  case AST_NIL:
    info.copy = true;
    return info;
  case AST_STRING:
    info.copy = false;
    return info;
  case AST_UNARYOP:
    return check_unary(node);
  case AST_BINOP:
    check_expr(node->data.binop.left, ExprUse::Read);
    check_expr(node->data.binop.right, ExprUse::Read);
    return info;
  case AST_CALL:
    return check_call(node);
  case AST_EXPRESSION_LIST:
    return check_expression_list(node, use);
  case AST_INDEX: {
    ExprInfo target = check_expr(node->data.index.target, ExprUse::Read);
    check_expr(node->data.index.index, ExprUse::Read);
    info.borrow_sources = target.borrow_sources;
    return info;
  }
  case AST_MEMBER_ACCESS: {
    ExprInfo obj = check_expr(node->data.member_access.object, ExprUse::Read);
    info.borrow_sources = obj.borrow_sources;
    return info;
  }
  case AST_STRUCT_LITERAL:
    return check_struct_literal(node);
  case AST_IF:
    check_stmt(node); // if expression handled as statement
    return info;
  case AST_FUNCTION:
    check_function(node);
    info.copy = true;
    return info;
  case AST_TOINT:
    check_expr(node->data.toint.expr, ExprUse::Read);
    return info;
  case AST_TOFLOAT:
    check_expr(node->data.tofloat.expr, ExprUse::Read);
    return info;
  case AST_INPUT:
    check_expr(node->data.input.prompt, ExprUse::Read);
    return info;
  default:
    return info;
  }
}

/**
 * @brief Check an identifier expression
 * @details Handle variable read, move, borrow semantics.
 *          **Fixed**: Using a mutably borrowed variable in any non‑assignment
 *          context is now forbidden.
 */
ExprInfo OwnershipChecker::check_identifier(ASTNode *node, ExprUse use) {
  ExprInfo info;
  info.type = node->inferred_type;
  info.copy = is_copy_type(node->inferred_type);
  const char *cname = node->data.identifier.name;
  if (!cname)
    return info;
  VarState *state = lookup(cname);
  if (!state)
    return info;
  info.type = state->type ? state->type : node->inferred_type;
  info.copy = is_copy_type(info.type);
  // Convention: names starting with uppercase are considered constants/types,
  // default copyable (simplified handling)
  if (std::isupper(static_cast<unsigned char>(cname[0])))
    info.copy = true;
  info.borrow_sources = state->borrow_sources;

  // A mutably borrowed variable cannot be used at all (except as the target of
  // an assignment, which is handled separately in check_assign)
  if (use != ExprUse::AssignTarget && state->borrowed_mut) {
    report(node, "cannot use '" + std::string(cname) +
                     "' while it is mutably borrowed");
    return info;
  }

  // Check if using a moved value (unless as assignment target)
  if (use != ExprUse::AssignTarget && state->moved) {
    std::string msg = std::string("use of moved value '") + cname + "'";
    if (state->moved_at) {
      msg += "\n  note: '" + std::string(cname) +
             "' was moved here by passing to " +
             std::string(node_file(state->moved_at)) + ":" +
             std::to_string(node_line(state->moved_at));
    }
    adjust_column_to_identifier(cname);
    adjust_column_to_identifier(cname);
    report(node, msg);
    return info;
  }
  // Move semantics: if non-Copy type used with Move, mark as moved
  if (use == ExprUse::Move && !info.copy) {
    log_var_modification(state);
    if (state->borrowed_shared || state->borrowed_mut) {
      report(node,
             std::string("cannot move '") + cname + "' while it is borrowed");
    } else {
      state->moved = true;
      state->moved_at = node;
      state->borrow_sources
          .clear(); // After move borrow relationships disappear
    }
  }
  return info;
}

/**
 * @brief Check a unary operation
 * @details Focus on handling borrow rules for take-address (&) and dereference
 * (*).  **Fixed**: When taking the address of a dereference, all possible
 * pointer targets are collected and checked.
 */
ExprInfo OwnershipChecker::check_unary(ASTNode *node) {
  ExprInfo info;
  info.type = node->inferred_type;
  info.copy = is_copy_type(node->inferred_type);
  UnaryOpType op = node->data.unaryop.op;
  if (op == OP_ADDRESS) {
    // Take address operation: check if borrowable
    std::string base = lvalue_base(node->data.unaryop.expr);
    if (!base.empty()) {
      std::unordered_set<std::string> roots;
      bool is_deref_operand =
          (node->data.unaryop.expr &&
           node->data.unaryop.expr->type == AST_UNARYOP &&
           node->data.unaryop.expr->data.unaryop.op == OP_DEREF);
      if (is_deref_operand) {
        // Expand all pointer levels to get the ultimate set of lvalues
        roots = resolve_pointer_targets(base, -1);
      } else {
        roots = {base};
      }

      // Check every possible root
      bool error_reported = false;
      for (const auto &root : roots) {
        VarState *root_state = lookup(root);
        if (!root_state)
          continue;
        log_var_modification(root_state);
        if (root_state->moved) {
          if (!error_reported) {
            report(node, "cannot borrow moved value '" + root + "'");
            error_reported = true;
          }
        }
        bool wants_mut = node->mutability == MUTABILITY_MUTABLE;
        if (wants_mut) {
          if (root_state->borrowed_shared || root_state->borrowed_mut) {
            if (!error_reported) {
              report(node,
                     "cannot mutably borrow '" + root + "' more than once");
              error_reported = true;
            }
          }
        } else {
          if (root_state->borrowed_mut) {
            if (!error_reported) {
              report(node, "cannot immutably borrow '" + root +
                               "' while it is mutably borrowed");
              error_reported = true;
            }
          }
        }
      }

      // If no error, apply the borrow flags to all roots
      if (!error_reported) {
        for (const auto &root : roots) {
          VarState *root_state = lookup(root);
          if (!root_state)
            continue;
          bool wants_mut = node->mutability == MUTABILITY_MUTABLE;
          if (wants_mut) {
            root_state->borrowed_mut = true;
          } else {
            root_state->borrowed_shared = true;
          }
        }
        info.borrow_sources = roots;
      }
    }
    check_expr(node->data.unaryop.expr, ExprUse::Address);
    info.is_ref = true;
    info.copy = true;
    return info;
  }
  if (op == OP_DEREF) {
    // Dereference: pass through borrow sources
    ExprInfo inner = check_expr(node->data.unaryop.expr, ExprUse::Read);
    info.borrow_sources = inner.borrow_sources;
    return info;
  }
  check_expr(node->data.unaryop.expr, ExprUse::Read);
  return info;
}

/**
 * @brief Check a function call
 * @details Handle argument passing (move semantics); if the function returns a
 * reference, collect borrow sources
 */
ExprInfo OwnershipChecker::check_call(ASTNode *node) {
  ExprInfo info;
  info.type = node->inferred_type;
  info.copy = is_copy_type(node->inferred_type);
  check_expr(node->data.call.func, ExprUse::Read);
  ASTNode *args = node->data.call.args;
  bool returns_ref =
      node->inferred_type && node->inferred_type->kind == TYPEINFO_PTR;
  if (args && args->type == AST_EXPRESSION_LIST) {
    for (int i = 0; i < args->data.expression_list.expression_count; i++) {
      ExprInfo arg_info =
          check_expr(args->data.expression_list.expressions[i], ExprUse::Move);
      if (returns_ref) {
        info.borrow_sources.insert(arg_info.borrow_sources.begin(),
                                   arg_info.borrow_sources.end());
      }
    }
  }
  return info;
}

/**
 * @brief Check an expression list (e.g., argument list, struct field list)
 */
ExprInfo OwnershipChecker::check_expression_list(ASTNode *node, ExprUse use) {
  ExprInfo info;
  info.type = node->inferred_type;
  info.copy = is_copy_type(node->inferred_type);
  for (int i = 0; i < node->data.expression_list.expression_count; i++) {
    ExprInfo elem = check_expr(node->data.expression_list.expressions[i], use);
    info.borrow_sources.insert(elem.borrow_sources.begin(),
                               elem.borrow_sources.end());
  }
  return info;
}

/**
 * @brief Check a struct literal
 * @details Each field is processed as move, collecting all borrow sources
 */
ExprInfo OwnershipChecker::check_struct_literal(ASTNode *node) {
  ExprInfo info;
  info.type = node->inferred_type;
  info.copy = false;
  ASTNode *fields = node->data.struct_literal.fields;
  if (fields && fields->type == AST_EXPRESSION_LIST) {
    for (int i = 0; i < fields->data.expression_list.expression_count; i++) {
      ASTNode *field = fields->data.expression_list.expressions[i];
      ExprInfo elem;
      if (field && field->type == AST_ASSIGN) {
        elem = check_expr(field->data.assign.right, ExprUse::Move);
      } else {
        elem = check_expr(field, ExprUse::Move);
      }
      info.borrow_sources.insert(elem.borrow_sources.begin(),
                                 elem.borrow_sources.end());
    }
  }
  return info;
}

/**
 * @brief Compute the number of dereference levels of an expression
 */
int OwnershipChecker::count_derefs(ASTNode *node) {
  int count = 0;
  while (node && node->type == AST_UNARYOP &&
         node->data.unaryop.op == OP_DEREF) {
    ++count;
    node = node->data.unaryop.expr;
  }
  return count;
}

/**
 * @brief Extract the underlying variable name of an expression
 */
std::string OwnershipChecker::lvalue_base(ASTNode *node) {
  if (!node)
    return {};
  switch (node->type) {
  case AST_IDENTIFIER:
    return node->data.identifier.name ? node->data.identifier.name : "";
  case AST_MEMBER_ACCESS:
    return lvalue_base(node->data.member_access.object);
  case AST_INDEX:
    return lvalue_base(node->data.index.target);
  case AST_UNARYOP:
    if (node->data.unaryop.op == OP_DEREF)
      return lvalue_base(node->data.unaryop.expr);
    return {};
  default:
    return {};
  }
}

/**
 * @brief Traverse the AST root node
 */
void OwnershipChecker::check_node(ASTNode *node) {
  if (!node)
    return;
  switch (node->type) {
  case AST_PROGRAM:
    check_block(node, false);
    end_statement();
    break;
  case AST_FUNCTION:
    check_function(node);
    break;
  case AST_STRUCT_DEF:
  case AST_IMPORT:
    break;
  case AST_GLOBAL:
    check_global(node);
    end_statement();
    break;
  default:
    check_stmt(node);
    break;
  }
}

} // namespace

/**
 * @brief C interface: exposed ownership checking entry point
 * @param root AST root node
 * @return 0 success, 1 failure
 */
extern "C" int ownership_check_program(ASTNode *root) {
  OwnershipChecker checker;
  return checker.check(root) == 0 ? 0 : 1;
}
