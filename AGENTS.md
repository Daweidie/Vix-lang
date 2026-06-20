# AGENTS.md — Vix Compiler Development Guide

## Project Overview

Vix is a compiled systems programming language targeting LLVM. This repository contains:

| Component | Language | Description |
|-----------|----------|-------------|
| `vixc` (host compiler) | C + C++ | Native compiler using Flex/Bison + LLVM C++ API + LLD |
| `vixc0` (bootstrap) | Vix | Self-hosted compiler proving the language is self-hosting |
| Tests | Python (pytest) | Regression, feature, fuzz, stress, CLI, and bootstrap test suites |

**Version**: 0.4.2  |  **LLVM**: 22.1.2(8)  |  **Build**: CMake

---

## Build & Test Commands

```bash
# Build vixc (from project root)
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run all tests
cd tests && python3 run.py

# Run a specific test category
python3 -m pytest tests/ -v --tb=short
python3 -m pytest tests/cli.py -v
python3 -m pytest tests/regre.py -v
python3 -m pytest tests/examp.py -v

# Build and test bootstrap (vixc0)
cd bootstrap && make test
```

---

## Architecture

### Compiler Pipeline (src/main.c)

```
Source (.vix) → Lex (flex) → Parse (bison) → Inline Imports
→ Semantic Check → Type Check (C++, HM inference) → Ownership Check
→ Codegen (LLVM IR) → Llc (.o) → Link (LLD) → Binary
```

### Key Source Files

#### C++ Codegen (src/compiler/)
| File | Lines | Role |
|------|-------|------|
| `Codegen.cpp` | 860 | Top-level orchestrator, scope, generic instantiation |
| `Exprs.cpp` | 914 | All expression codegen (binary ops, calls, member access, literals) |
| `Stmts.cpp` | 1204 | Statement codegen (let, if, while, for, match, return) |
| `Funcs.cpp` | 1383 | Function declarations, calls, extern, generics |
| `Structs.cpp` | 953 | Struct/ADT definitions, literals, member access |
| `Arrays.cpp` | 450 | Array indexing, literals, .length |
| `Types.cpp` | 485 | Vix type ↔ LLVM type mapping, casts, promotions |

#### C++ Type Checker (src/Typeck/)
| File | Lines | Role |
|------|-------|------|
| `Typeck.cpp` | 2660 | Full HM type inference, compatibility checks, generics |
| `LayOut.cpp` | 155 | Struct layout, sizeof, alignof |

#### C Frontend (src/)
| File | Lines | Role |
|------|-------|------|
| `ast/ast.c` | 2462 | AST construction, printing, freeing, import inlining |
| `semantic/semantic.c` | 1458 | Symbol table, undefined checks, unused variable detection |
| `utils/error.c` | 928 | Rich diagnostics (ANSI color, source context, carets) |
| `parser/parser.y` | 1953 | Bison grammar |
| `parser/lexer.l` | 321 | Flex lexer |

### Bootstrap Compiler (bootstrap/)

All written in Vix, compiled by `vixc`:

| File | Lines | Role |
|------|-------|------|
| `main.vix` | 195 | CLI dispatch: `--lex`, `--parser`, `--ast`, `--typeinfer`, `--semantic`, `codegen` |
| `lexer.vix` | 440 | Hand-written lexer (TokenStream) |
| `parser.vix` | 917 | Recursive descent parser |
| `ast.vix` | 787 | AST definitions (ExprStore/StmtStore tagged unions) |
| `typeinfer.vix` | 611 | Type inference |
| `semantic.vix` | 253 | Semantic utilities |
| `codegen.vix` | 496 | LLVM IR emission |
| `builder.vix` | 669 | LLVM-C API wrappers |
| `error.vix` | 361 | Diagnostic system |
| `helper.c` | 477 | C glue layer for LLVM-C bridge |

### Tests (tests/)

| File | Tests | Description |
|------|-------|-------------|
| `regre.py` | 400+ | Regression: compiles `tests/regression/test*.vix`, checks expected output |
| `feat.py` | 500+ | Feature tests in 24 classes (arithmetic, control flow, strings, match, structs, ADTs, generics…) |
| `fuzz.py` | 470 | Random program generation, checks no crash |
| `stress.py` | 120+ | Deep nesting, large programs |
| `cli.py` | 15 | CLI flags: -v, -h, -o, -ll, -ast, -S, -obj, -opt |
| `errors.py` | 15 | Error handling tests |
| `examp.py` | ~10 | Compiles examples from `WORKING_EXAMPLES` list |
| `c0_test.py` | 66 | Bootstrap compiler tests |

---

## Conventions

### Code Style

- **C**: C11, `snake_case` functions, `PascalCase` types, 2-space indent
- **C++**: C++17, `PascalCase` classes/methods, `camelCase` members, LLVM coding style
- **Vix**: 4-space indent, `snake_case` functions/variables, `PascalCase` types/ADTs

### Testing

- New regression tests go in `tests/regression/` as `test{N}.vix` files
- Add expected output to `EXPECT` dict in `tests/regre.py`: `(True, ["expected", "output"])` for success, `(False, "error message substring")` for failure
- New example tests: add filename to `WORKING_EXAMPLES` list in `tests/examp.py`
- Feature tests: add to `tests/feat.py` under the appropriate test class
- CLI tests: add to `tests/cli.py`
- Run `python3 tests/run.py` to execute the full suite

### AST Node Creation

Use `create_*_node()` or `create_*_node_loc()` for location-attached nodes. See `include/ast.h` for all node types.

### Error Reporting

Use `report_*_error_with_location()` from `include/compiler.h`. Error codes:
- E1xxx: Parse errors
- E2xxx: Name/undefined errors
- E3xxx: Type errors

---

## Known Issues & Work Items

1. **CTFE bug**: In small functions, rebinding `[string]` fields on imported structs passed by value produces incorrect evaluation. Look in `src/compiler/Exprs.cpp` (member access) and `src/compiler/Structs.cpp` (struct value passing/copying).

2. **Example coverage**: Only 16/53 examples compile (30%). Target: 85% (45+).

3. **Source location tracking**: Many AST nodes lack source location for error reporting. Use `create_*_node_loc()` variants and ensure `Location` is propagated.

4. **Bootstrap decoupling**: `ast.vix` and `codegen.vix` need separation of concerns. `bootstrap/main.vix` needs more CLI parameters (output file, optimization level, target selection).

---

## Directory Map

```
Vix-lang/
├── bootstrap/          # Self-hosted compiler (Vix source, must not modify for vixc bugs)
│   ├── main.vix        # Entry point, CLI parsing
│   ├── parser.vix      # Recursive descent parser
│   ├── lexer.vix       # Hand-written lexer
│   ├── ast.vix         # AST types and debug printing
│   ├── codegen.vix     # LLVM IR codegen (coupled with ast.vix)
│   ├── typeinfer.vix   # Type inference
│   ├── semantic.vix    # Semantic checks
│   ├── builder.vix     # LLVM-C API wrappers
│   ├── error.vix       # Error diagnostics
│   ├── helper.c        # C glue layer
│   ├── build.sh        # Build script
│   └── makefile        # Make targets (all, test, clean)
├── src/
│   ├── main.c          # vixc entry point
│   ├── compiler/       # C++: Codegen (Exprs, Stmts, Funcs, Structs, Arrays, Types, Passes)
│   │   ├── Llc/        # LLVM IR → object file
│   │   └── Linker/     # Object file → executable (LLD)
│   ├── Typeck/         # C++: Type checking (HM inference), Layout
│   ├── Ownership/      # C++: Move/borrow checking
│   ├── parser/         # Flex lexer + Bison grammar
│   ├── ast/            # AST construction, type inference (C)
│   ├── semantic/       # Symbol table, semantic analysis (C)
│   └── utils/          # Error diagnostics (C)
├── include/            # Public headers (ast.h, type.h, unify.h, env.h, codegen.h…)
├── std/                # Standard library (Vix source: io, String, mem, arr, hash, json, net…)
├── tests/
│   ├── regression/     # 407 .vix regression test files
│   ├── vixc0/          # 12 .vix bootstrap test files
│   ├── *.py            # Test runner + test modules
│   └── run.py          # Unified test runner
├── examples/           # 53 example .vix programs
├── docs/               # Release notes
├── CMakeLists.txt      # CMake build (requires LLVM 22, Bison 3, Flex 2.6)
├── Makefile            # `make test` → builds and tests bootstrap
└── pyproject.toml      # Pytest configuration
```
