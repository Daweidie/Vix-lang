# vixc Compiler Internals

This document describes the architecture, compilation pipeline, and source code organization of the **vixc** compiler for the Vix programming language.

## Architecture Overview

vixc is a native ahead-of-time compiler that translates Vix source files (`.vix`) into platform executables. It is implemented in C (frontend) and C++ (backend), uses Flex/Bison for lexing/parsing, and targets native code via LLVM.

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│  Flex Lexer  │───>│ Bison Parser │───>│    AST       │───>│   Import     │
│  (lexer.l)   │    │ (parser.y)   │    │  (ast.c)     │    │  Inlining   │
└──────────────┘    └──────────────┘    └──────────────┘    └──────┬───────┘
                                                                   │
┌──────────────┐    ┌──────────────┐    ┌──────────────┐           │
│  LLD Linker  │<───│  LLC/Object  │<───│  LLVM IR     │<──────────┘
│ (Linker.cpp) │    │  (Llc.cpp)   │    │(CodeGen.cpp) │    ┌──────────────┐
└──────────────┘    └──────────────┘    └──────────────┘    │   Semantic   │
                                                            │  Analysis    │
                                                            │(semantic.c)  │
                                                            └──────┬───────┘
                                                                   │
                                                            ┌──────┴───────┐
                                                            │ Type Checking│
                                                            │(Typeck.cpp)  │
                                                            └──────────────┘
```

## Compilation Pipeline

### 1. Lexing (`src/parser/lexer.l`)

The Flex-based lexer tokenizes Vix source into tokens:
- **Keywords:** `fn`, `let`, `mut`, `if`, `elif`, `else`, `while`, `for`, `in`, `return`, `match`, `struct`, `type`, `import`, `extern`, `pub`, `break`, `continue`, `true`, `false`, `nil`, `and`, `or`
- **Types:** `i8`, `i32`, `i64`, `f32`, `f64`, `bool`, `string`, `str`, `void`, `ptr`
- **Literals:** integers (decimal, hex `0x`, octal `0o`, binary `0b`), floats, strings, chars, booleans
- **Operators:** arithmetic (`+`, `-`, `*`, `/`, `%`, `**`), comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`), logical (`and`, `or`), pointer (`&`, `@`), compound assignment (`+=`, `-=`, `*=`, `/=`)
- **Comments:** line (`//`) and block (`/* */`)

### 2. Parsing (`src/parser/parser.y`)

The Bison grammar builds an AST from the token stream. Key productions:
- **Program:** sequence of top-level declarations (imports, globals, structs, types, functions, externs)
- **Functions:** `fn name<generics>(params) -> RetType { body }`
- **Structs:** `struct Name<generics> { field: Type, ... }`
- **ADTs:** `type Name<generics> = Ctor1 | Ctor2(Type) | ...`
- **Statements:** variable declarations, assignments, if/elif/else, while, for-in-range, return, match
- **Expressions:** full operator precedence chain from logical-or down to unary/postfix

### 3. Import Inlining (`src/ast/ast.c`)

The `inline_imports()` function recursively:
1. Resolves `import "path"` statements
2. Parses the imported `.vix` file
3. Inlines all `pub`-marked functions into the current AST
4. Handles circular imports via visited-file tracking

### 4. Semantic Analysis (`src/semantic/semantic.c`)

Performs scope-aware validation:
- **Symbol table:** scoped linked-list with parent chain for lexical scoping
- **Undefined identifiers:** reports use of undeclared variables/functions
- **Unused variables:** warns about declared but unused variables
- **Redefinition detection:** catches duplicate variable declarations in the same scope
- **Self-recursive structs:** prevents infinite-size struct definitions (e.g., `struct Node { next: Node }`)

### 5. Type Checking (`src/Typeck/Typeck.cpp`)

Implements Hindley-Milner type inference with unification:
- **Type representation:** tagged union (`TypeKind`) with variants for all Vix types
- **Unification:** `Unifier` class performs type unification with occurs check
- **Generic instantiation:** creates fresh type variables for generic parameters and specializes at call sites
- **ADT/Result/Option:** built-in support for algebraic data types
- **Match exhaustiveness:** verifies all constructors are covered in match expressions
- **Layout computation** (`LayOut.cpp`): computes `sizeof` and `alignof` for all types

### 6. LLVM Code Generation (`src/compiler/CodeGen.cpp`)

Translates the typed AST to LLVM IR:
- **Functions:** creates LLVM function signatures, manages basic blocks
- **Structs:** computes field offsets, generates GEP instructions for field access
- **Arrays/Lists:** fixed-size arrays as LLVM arrays, dynamic lists as pointers with `.length` metadata
- **Pointers:** address-of (`&`) and dereference (`@`) via LLVM `alloca`/`load`/`store`
- **Match:** generates LLVM `switch` or cascading `if/else` blocks
- **ADT constructors:** tagged union representation with constructor discriminant
- **Generics:** monomorphization — each unique type instantiation gets its own LLVM function/struct
- **String literals:** global constant string pointers
- **Built-in functions:** `print`, `toint`, `tofloat` are emitted inline

### 7. Optimization (`src/compiler/Passes.cpp`)

LLVM optimization passes at configurable levels:
- `-opt=l0`: No optimization (default)
- `-opt=l1`: `-O1` passes
- `-opt=l2`: `-O2` passes
- `-opt=l3`: `-O3` passes (includes LTO)

### 8. Object Emission (`src/compiler/Llc/Llc.cpp`)

Compiles LLVM IR to object code or assembly:
- Supports x86_64, AArch64, ARM, RISC-V, WebAssembly targets
- `-S` flag emits assembly, `-obj` emits object file

### 9. Linking (`src/compiler/Linker/Linker.cpp`)

Uses LLD for linking:
- **Linux:** ELF format
- **macOS:** MachO format
- **Windows:** COFF/MinGW format
- **WebAssembly:** Wasm format
- Bundles C runtime startup (`crt1.o`, `crti.o`, `crtn.o`) and links `-lc`, `-lm`, `-lpthread`

## Source Code Organization

```
src/
├── main.c                    # Entry point, CLI argument parsing, pipeline orchestration
├── ast/
│   ├── ast.c                 # AST node creation, manipulation, printing, import inlining
│   └── typeinfer.c           # Legacy C-based type inference
├── parser/
│   ├── lexer.l               # Flex lexer definition
│   └── parser.y              # Bison grammar definition
├── semantic/
│   └── semantic.c            # Symbol table, scope analysis, undefined/unused checks
├── Typeck/
│   ├── Typeck.cpp            # Hindley-Milner type checker with unification
│   ├── TypeckInfer.cpp       # C-API bridge to C++ type checker
│   └── LayOut.cpp            # Type size/alignment computation
├── compiler/
│   ├── CodeGen.cpp           # LLVM IR generation from AST
│   ├── Passes.cpp            # LLVM optimization pass configuration
│   ├── Llc/
│   │   ├── Llc.h
│   │   └── Llc.cpp           # LLVM IR → object/assembly compilation
│   └── Linker/
│       ├── Linker.h
│       └── Linker.cpp        # LLD-based linking
├── utils/
│   └── error.c               # Error reporting with source context and ANSI coloring
└── std/                      # Standard library (written in Vix)
    ├── io.vix                # I/O: puts, fopen, fclose, fread, fwrite, panic
    ├── arr.vix               # Array operations: sort
    ├── strings.vix           # String operations: strcmp
    ├── mem.vix               # Memory: malloc, free, memcpy, memset, memcmp
    ├── os.vix                # OS: system, exit, opendir, readdir, closedir
    ├── rand.vix              # Random number generation
    └── net.vix               # Networking: socket, listen, accept, send, close
```

### Header Files (`include/`)

| Header | Purpose |
|--------|---------|
| `ast.h` | AST node types (40+ variants), TypeInfo, creation/clone functions |
| `codegen.h` | `llvm_emit_from_ast()`, target triple configuration |
| `compiler.h` | Error reporting API, source location tracking |
| `compat.h` | Cross-platform compatibility (Win32/POSIX) |
| `env.h` | `TypeEnv` class: scoped value/struct/ADT/constructor environment |
| `parser.h` | `yylex()`, `yyparse()` declarations |
| `semantic.h` | `SymbolTable`, `Symbol`, semantic analysis functions |
| `type.h` | `Type` struct with tagged union, factory methods |
| `typeck.h` | `typecheck_program()` public API |
| `typeckinternal.h` | Internal type checking API |
| `typeinfer.h` | Legacy C type inference context |
| `unify.h` | `Unifier` class: Hindley-Milner unification engine |

## Build System

### Prerequisites

- CMake 3.20+
- C11 compiler (GCC or Clang)
- C++17 compiler
- Flex 2.6+
- Bison 3.0+
- LLVM 18-20 (with development headers)
- LLD 18-20

### Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

### Installing Dependencies

```bash
# Ubuntu/Debian
sudo apt install cmake flex bison llvm-dev lld

# Arch Linux
sudo pacman -S cmake flex bison llvm lld

# macOS (Homebrew)
brew install cmake flex bison llvm lld
```

Or use the provided installer: `bash src/install.sh`

## Compiler CLI

```
USAGE: vixc [options] <input.vix>

OPTIONS:
  -o <file>              Write output to <file>
  -S [file]              Emit assembly (default: <input>.s)
  -obj [file]            Emit object file (default: <input>.o)
  -ll [file]             Emit LLVM IR (default: <input>.ll)
  -llvm                  Print LLVM IR to stdout
  -ast                   Print AST to stdout
  -opt=lN                Set optimization level (N = 0..3)
  --target=<triple>      Set codegen/link target triple
  --debug                Enable debug output
  -v, --version          Display compiler version
  -h, --help             Display help message
```

## Testing

The test suite uses pytest and includes:

```bash
# Run all tests
.venv/bin/python -m pytest tests/ -v

# Run specific categories
.venv/bin/python -m pytest tests/ -m unit        # Unit tests
.venv/bin/python -m pytest tests/ -m integration  # Integration tests
.venv/bin/python -m pytest tests/ -m feature      # Feature tests
.venv/bin/python -m pytest tests/ -m fuzz         # Fuzz tests
.venv/bin/python -m pytest tests/ -m stress       # Stress tests
.venv/bin/python -m pytest tests/ -m cli          # CLI tests
.venv/bin/python -m pytest tests/ -m error        # Error handling tests

# Run the legacy test runner
python test/run.py

# Run C unit tests (build first)
gcc -I include test/unit_tests.c -o build/unit_tests -I build/parser
./build/unit_tests

# Run fuzz tests (standalone)
python test/fuzz/fuzz.py
```

### Test Structure

```
tests/
├── conftest.py         # pytest fixtures and configuration
├── helpers.py          # Shared test utilities
├── test_regression.py  # 212 regression tests (compile + run + verify output)
├── test_features.py    # Feature-specific tests (arithmetic, control flow, structs, etc.)
├── test_errors.py      # Error handling and diagnostics tests
├── test_cli.py         # Command-line interface tests
├── test_stress.py      # Stress tests (deep nesting, large programs, loops)
├── test_fuzz.py        # Fuzz testing (random/malformed programs)
├── test_unit.py        # Unit tests (source structure, headers, stdlib)
└── test_examples.py    # Example program compilation tests
```

## Cross-Compilation

vixc supports cross-compilation via the `--target` flag:

```bash
# Compile for ARM64 Linux
vixc input.vix --target=aarch64-linux-gnu -o output

# Compile for Windows
vixc input.vix --target=x86_64-w64-mingw32 -o output.exe

# Compile for WebAssembly
vixc input.vix --target=wasm32-unknown-unknown -o output.wasm
```

## Error Reporting

vixc provides rich error diagnostics with:
- Error category (Syntax, Lex, Type, Undefined, Redefinition, Semantic)
- Source location (file, line, column)
- Source context with caret pointing to the error
- Help suggestions for fixing the error
- ANSI color output on terminal devices

## Testing

### Running Tests

```bash
# Activate the Python virtual environment
source .venv/bin/activate

# Run all tests
python -m pytest tests/test_features.py tests/test_fuzz.py tests/test_stress.py -v

# Run specific test suites
python -m pytest tests/test_features.py -v    # 500+ feature tests
python -m pytest tests/test_fuzz.py -v        # 500+ fuzz tests
python -m pytest tests/test_stress.py -v      # 100+ stress tests

# Run existing regression tests
python -m pytest tests/regre.py -v

# Run the full test runner
python tests/run.py
```

### Test Organization

| File | Category | Description |
|------|----------|-------------|
| `test_features.py` | `@pytest.mark.feature` | 500+ feature tests covering all language features |
| `test_fuzz.py` | `@pytest.mark.fuzz` | 500+ fuzz tests generating random valid programs |
| `test_stress.py` | `@pytest.mark.stress` | 100+ stress tests for edge cases and large programs |
| `feat.py` | `@pytest.mark.feature` | Additional feature tests |
| `fuzz.py` | `@pytest.mark.fuzz` | Additional fuzz tests |
| `stress.py` | `@pytest.mark.stress` | Additional stress tests |
| `errors.py` | `@pytest.mark.error` | Error handling and diagnostics tests |
| `examp.py` | `@pytest.mark.integration` | Example file compilation tests |
| `regre.py` | regression | 220+ regression tests against known-good outputs |

## Changelog

### v0.1.2 (2026-05-15)

#### New Features
- **Power operator (`**`):** Full implementation of integer and floating-point exponentiation. Integer power uses an efficient loop-based algorithm; float power delegates to libc `pow()`. Constant expressions are folded at compile time.
- **String pattern matching:** `match` expressions now correctly compare strings using `strcmp()` instead of pointer equality. Patterns like `match s { "hello" -> ... }` work as expected.
- **Type annotation enforcement:** `let x: T = value` now properly validates that the initializer's type matches the declared type. Numeric types (i8, i32, i64, f32, f64) are allowed to promote. Non-numeric type mismatches produce clear error messages.
- **ADT constructor type inference:** User-defined ADT constructors with payloads are now properly type-checked. The `check_call` function resolves registered constructors via `env.lookup_ctor()` before falling back to regular function lookup.

#### Bug Fixes
- Fixed `OP_POW` missing from `visitBinOp()` in CodeGen.cpp — power expressions now generate correct LLVM IR
- Fixed `OP_POW` missing from `evaluateConstExpr()` — constant power expressions are now folded
- Fixed string comparison in `visitBinOp()` — string equality (`==`, `!=`, `<`, `>`, `<=`, `>=`) now uses `strcmp()` instead of pointer comparison
- Fixed `check_assign()` in Typeck.cpp silently swallowing type mismatch errors via empty `catch (...) {}`
- Improved ADT constructor resolution in `check_call()` with dedicated `env.lookup_ctor()` path

#### Test Suite
- Added 500+ feature tests (`tests/test_features.py`) covering arithmetic, variables, control flow, functions, strings, match, types, structs, arrays, pointers, ADTs, generics
- Added 500+ fuzz tests (`tests/test_fuzz.py`) generating random valid programs to verify crash-freedom
- Added 100+ stress tests (`tests/test_stress.py`) for deep nesting, large programs, loop stress, expression complexity

#### Examples
- Added `examples/match_strings.vix` — string pattern matching demo
- Added `examples/power.vix` — power operator demo
- Added `examples/adt_pattern.vix` — ADT pattern matching with Option/Result

#### Documentation
- Updated `Docs/COMPILER.md` with testing section and changelog
- Updated `Docs/syntax.ebnf` with power operator in expression grammar
