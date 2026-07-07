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

enum class ExprUse { Read, Move, Address, AssignTarget };

struct VarState {
  const TypeInfo *type = nullptr;
  bool is_mutable = false;
  bool moved = false;
  bool is_global = false;
  bool borrowed_shared = false;
  bool borrowed_mut = false;
  // Set of variables this variable borrows (e.g., struct containing multiple
  // references)
  std::unordered_set<std::string> borrow_sources;
};

struct Scope {
  std::unordered_map<std::string, VarState> vars;
};

struct ExprInfo {
  const TypeInfo *type = nullptr;
  bool copy = true;
  bool is_ref = false;
  // Propagated set of borrow sources
  std::unordered_set<std::string> borrow_sources;
};

// A snapshot of moved states across all scopes, indexed by scope depth.
using ScopeSnapshot = std::vector<std::unordered_map<std::string, bool>>;

class OwnershipChecker {
public:
  int check(ASTNode *root) {
    push_scope();
    check_node(root);
    pop_scope();
    return errors;
  }

private:
  std::vector<Scope> scopes;
  int errors = 0;

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

  void report(ASTNode *node, const std::string &message) {
    set_location_with_column(node_file(node), node_line(node), node_col(node));
    report_simple_error(ERROR_LEVEL_ERROR, ERROR_SEMANTIC, message.c_str());
    errors++;
  }

  VarState *lookup(const std::string &name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto found = it->vars.find(name);
      if (found != it->vars.end())
        return &found->second;
    }
    return nullptr;
  }

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

  static int count_derefs(ASTNode *node) {
    int count = 0;
    while (node && node->type == AST_UNARYOP &&
           node->data.unaryop.op == OP_DEREF) {
      ++count;
      node = node->data.unaryop.expr;
    }
    return count;
  }

  std::string trace_borrow_levels(const std::string &base, int levels) {
    if (levels <= 0 || base.empty())
      return base;
    std::string current = base;
    for (int i = 0; i < levels; ++i) {
      VarState *state = lookup(current);
      if (!state || state->borrow_sources.empty())
        break;
      if (state->type && state->type->kind == TYPEINFO_PTR) {
        // follow the first borrow source (they are all equivalent for a single
        // reference)
        current = *state->borrow_sources.begin();
      } else {
        break;
      }
    }
    return current;
  }

  void declare_var(const std::string &name, const TypeInfo *type,
                   bool is_mutable, bool is_global,
                   const std::unordered_set<std::string> &borrow_sources = {}) {
    if (scopes.empty())
      push_scope();
    VarState state;
    state.type = type;
    state.is_mutable = is_mutable;
    state.is_global = is_global;
    state.borrow_sources = borrow_sources;
    scopes.back().vars[name] = std::move(state);
  }

  void end_statement() {
    // Collect all borrow source names that are still alive across all scopes.
    std::unordered_set<std::string> active_borrow_sources;
    for (auto &scope : scopes) {
      for (auto &entry : scope.vars) {
        for (const auto &src : entry.second.borrow_sources) {
          active_borrow_sources.insert(src);
        }
      }
    }
    // Clear borrow flags for variables that are no longer referenced by any
    // alive variable.
    for (auto &scope : scopes) {
      for (auto &entry : scope.vars) {
        if (active_borrow_sources.find(entry.first) ==
            active_borrow_sources.end()) {
          entry.second.borrowed_shared = false;
          entry.second.borrowed_mut = false;
        }
      }
    }
  }

  ScopeSnapshot capture_moved_snapshot() {
    ScopeSnapshot snap;
    snap.reserve(scopes.size());
    for (const auto &scope : scopes) {
      std::unordered_map<std::string, bool> m;
      for (const auto &entry : scope.vars) {
        m[entry.first] = entry.second.moved;
      }
      snap.push_back(std::move(m));
    }
    return snap;
  }

  void apply_moved_snapshot(const ScopeSnapshot &snap) {
    size_t n = std::min(scopes.size(), snap.size());
    for (size_t i = 0; i < n; ++i) {
      for (const auto &p : snap[i]) {
        auto it = scopes[i].vars.find(p.first);
        if (it != scopes[i].vars.end()) {
          it->second.moved = p.second;
        }
      }
    }
  }

  ScopeSnapshot merge_moved_snapshots(const ScopeSnapshot &then_snap,
                                      const ScopeSnapshot &else_snap,
                                      bool has_else) {
    size_t depth = then_snap.size();
    if (has_else) {
      depth = std::max(depth, else_snap.size());
    }
    ScopeSnapshot result(depth);
    for (size_t i = 0; i < depth; ++i) {
      std::unordered_set<std::string> keys;
      if (i < then_snap.size()) {
        for (const auto &p : then_snap[i])
          keys.insert(p.first);
      }
      if (has_else && i < else_snap.size()) {
        for (const auto &p : else_snap[i])
          keys.insert(p.first);
      }
      for (const auto &k : keys) {
        bool in_then = false;
        if (i < then_snap.size()) {
          auto it = then_snap[i].find(k);
          if (it != then_snap[i].end())
            in_then = it->second;
        }
        bool in_else = false;
        if (has_else && i < else_snap.size()) {
          auto it = else_snap[i].find(k);
          if (it != else_snap[i].end())
            in_else = it->second;
        }
        result[i][k] = has_else ? (in_then || in_else) : in_then;
      }
    }
    return result;
  }

  std::string lvalue_base(ASTNode *node) {
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

  void check_node(ASTNode *node) {
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

  void check_block(ASTNode *block, bool new_scope) {
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
      end_statement(); // Release borrows held by variables destroyed on scope
                       // exit
    }
  }

  void check_function(ASTNode *fn) {
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
        if (param->type == AST_ASSIGN) {
          id = param->data.assign.left;
          type = param->inferred_type
                     ? param->inferred_type
                     : (param->data.assign.right
                            ? param->data.assign.right->inferred_type
                            : nullptr);
          mut = param->data.assign.mutability == MUTABILITY_MUTABLE ||
                (id && id->mutability == MUTABILITY_MUTABLE);
          // Evaluate default value in a temporary scope using Read to avoid
          // false moves.
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

  void check_global(ASTNode *node) {
    if (!node || !node->data.global_decl.identifier ||
        node->data.global_decl.identifier->type != AST_IDENTIFIER) {
      return;
    }
    ExprInfo init =
        check_expr(node->data.global_decl.initializer, ExprUse::Read);
    const char *name = node->data.global_decl.identifier->data.identifier.name;
    if (name)
      declare_var(name, node->inferred_type ? node->inferred_type : init.type,
                  true, true, init.borrow_sources);
  }

  void check_stmt(ASTNode *stmt) {
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
      check_expr(stmt->data.if_stmt.condition, ExprUse::Read);
      end_statement();
      ScopeSnapshot saved_moved = capture_moved_snapshot();
      check_block(stmt->data.if_stmt.then_body, true);
      ScopeSnapshot then_moved = capture_moved_snapshot();
      ScopeSnapshot else_moved;
      bool has_else = (stmt->data.if_stmt.else_body != nullptr);
      if (has_else) {
        apply_moved_snapshot(saved_moved);
        check_block(stmt->data.if_stmt.else_body, true);
        else_moved = capture_moved_snapshot();
      }
      ScopeSnapshot merged =
          merge_moved_snapshots(then_moved, else_moved, has_else);
      apply_moved_snapshot(merged);
      return;
    }
    case AST_WHILE:
      check_expr(stmt->data.while_stmt.condition, ExprUse::Read);
      end_statement();
      check_block(stmt->data.while_stmt.body, true);
      // Moves inside the loop body persist after the loop (conservative).
      // TODO: continue statements may cause false positives because moved
      // states from code after continue are accumulated. A more precise
      // analysis would merge the moved state at every branch point (including
      // continue).
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
      // Moves inside the loop body persist after the loop.
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

  void check_assign(ASTNode *node) {
    ASTNode *left = node->data.assign.left;
    ASTNode *right = node->data.assign.right;
    bool is_decl = node->data.assign.is_declaration != 0;
    ExprInfo rhs = check_expr(right, ExprUse::Move);

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

    std::string base = lvalue_base(left);
    if (!base.empty()) {
      int deref_count = count_derefs(left);
      std::string target = trace_borrow_levels(base, deref_count);
      VarState *target_state = lookup(target);
      if (target_state) {
        if (target_state->moved)
          report(left, "use of moved value '" + target + "'");
        if (target_state->borrowed_shared || target_state->borrowed_mut) {
          report(left,
                 "cannot assign to '" + target + "' while it is borrowed");
        }
        target_state->moved = false;

        // Determine if the assignment replaces the entire variable or only a
        // part.
        bool is_partial = false;
        ASTNode *lv = left;
        while (lv && lv->type == AST_UNARYOP &&
               lv->data.unaryop.op == OP_DEREF) {
          lv = lv->data.unaryop.expr;
        }
        if (lv && (lv->type == AST_MEMBER_ACCESS || lv->type == AST_INDEX)) {
          is_partial = true;
        }
        if (is_partial) {
          // Partial assignment (e.g., s.field = ...) merges new borrow sources
          // to preserve existing borrows from other fields.
          // NOTE: This is conservative; old borrow sources from the same field
          // are never removed, which may cause false positives but ensures
          // safety.
          target_state->borrow_sources.insert(rhs.borrow_sources.begin(),
                                              rhs.borrow_sources.end());
        } else {
          target_state->borrow_sources = rhs.borrow_sources;
        }
      }
    }
    check_expr(left, ExprUse::AssignTarget);
  }

  void check_return(ASTNode *node) {
    ExprInfo value = check_expr(node->data.return_stmt.expr, ExprUse::Move);
    for (const auto &src : value.borrow_sources) {
      if (is_local_name(src)) {
        report(node->data.return_stmt.expr ? node->data.return_stmt.expr : node,
               "cannot return reference to local variable '" + src + "'");
        break;
      }
    }
  }

  ExprInfo check_expr(ASTNode *node, ExprUse use) {
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
      // NOTE: This treats field access as borrowing the whole object, like
      // Rust's field-level borrow. This is conservative and safe, though it may
      // cause false borrow conflicts.
      info.borrow_sources = obj.borrow_sources;
      return info;
    }
    case AST_STRUCT_LITERAL:
      return check_struct_literal(node);
    case AST_IF:
      check_stmt(node);
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

  ExprInfo check_identifier(ASTNode *node, ExprUse use) {
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
    if (std::isupper(static_cast<unsigned char>(cname[0])))
      info.copy = true;
    info.borrow_sources = state->borrow_sources;

    if (use != ExprUse::AssignTarget && state->moved) {
      report(node, std::string("use of moved value '") + cname + "'");
      return info;
    }
    if (use == ExprUse::Move && !info.copy) {
      if (state->borrowed_shared || state->borrowed_mut) {
        report(node,
               std::string("cannot move '") + cname + "' while it is borrowed");
      } else {
        state->moved = true;
        state->borrow_sources.clear();
      }
    }
    return info;
  }

  ExprInfo check_unary(ASTNode *node) {
    ExprInfo info;
    info.type = node->inferred_type;
    info.copy = is_copy_type(node->inferred_type);
    UnaryOpType op = node->data.unaryop.op;
    if (op == OP_ADDRESS) {
      std::string base = lvalue_base(node->data.unaryop.expr);
      if (!base.empty()) {
        std::string root = base;
        bool is_deref_operand =
            (node->data.unaryop.expr &&
             node->data.unaryop.expr->type == AST_UNARYOP &&
             node->data.unaryop.expr->data.unaryop.op == OP_DEREF);
        if (is_deref_operand) {
          std::unordered_set<std::string> visited;
          VarState *state = lookup(root);
          while (state && !state->borrow_sources.empty() && state->type &&
                 state->type->kind == TYPEINFO_PTR) {
            if (!visited.insert(root).second)
              break;
            // follow the first borrow source (should be single for a reference)
            root = *state->borrow_sources.begin();
            state = lookup(root);
          }
        }
        VarState *root_state = lookup(root);
        if (root_state) {
          if (root_state->moved)
            report(node, "cannot borrow moved value '" + root + "'");
          bool wants_mut = node->mutability == MUTABILITY_MUTABLE;
          if (wants_mut) {
            if (root_state->borrowed_shared || root_state->borrowed_mut) {
              report(node,
                     "cannot mutably borrow '" + root + "' more than once");
            }
            root_state->borrowed_mut = true;
          } else {
            if (root_state->borrowed_mut) {
              report(node, "cannot immutably borrow '" + root +
                               "' while it is mutably borrowed");
            }
            root_state->borrowed_shared = true;
          }
          // The new reference only borrows the root variable, not its entire
          // borrow set
          info.borrow_sources = {root};
        }
      }
      check_expr(node->data.unaryop.expr, ExprUse::Address);
      info.is_ref = true;
      info.copy = true;
      return info;
    }
    if (op == OP_DEREF) {
      ExprInfo inner = check_expr(node->data.unaryop.expr, ExprUse::Read);
      info.borrow_sources = inner.borrow_sources;
      return info;
    }
    check_expr(node->data.unaryop.expr, ExprUse::Read);
    return info;
  }

  ExprInfo check_call(ASTNode *node) {
    ExprInfo info;
    info.type = node->inferred_type;
    info.copy = is_copy_type(node->inferred_type);
    check_expr(node->data.call.func, ExprUse::Read);
    ASTNode *args = node->data.call.args;
    bool returns_ref =
        node->inferred_type && node->inferred_type->kind == TYPEINFO_PTR;
    if (args && args->type == AST_EXPRESSION_LIST) {
      for (int i = 0; i < args->data.expression_list.expression_count; i++) {
        ExprInfo arg_info = check_expr(
            args->data.expression_list.expressions[i], ExprUse::Move);
        if (returns_ref) {
          // Conservatively merge borrow sources from all arguments to ensure no
          // potential returned reference (from any argument) is missed.
          info.borrow_sources.insert(arg_info.borrow_sources.begin(),
                                     arg_info.borrow_sources.end());
        }
      }
    }
    return info;
  }

  ExprInfo check_expression_list(ASTNode *node, ExprUse use = ExprUse::Move) {
    ExprInfo info;
    info.type = node->inferred_type;
    info.copy = is_copy_type(node->inferred_type);
    for (int i = 0; i < node->data.expression_list.expression_count; i++) {
      ExprInfo elem =
          check_expr(node->data.expression_list.expressions[i], use);
      info.borrow_sources.insert(elem.borrow_sources.begin(),
                                 elem.borrow_sources.end());
    }
    return info;
  }

  ExprInfo check_struct_literal(ASTNode *node) {
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
};

} // namespace

extern "C" int ownership_check_program(ASTNode *root) {
  OwnershipChecker checker;
  return checker.check(root) == 0 ? 0 : 1;
}
