# Vix v0.2.2 Release Notes

**ADT Runtime Fixes & Syntax Enhancements Release**

## New Features

### 1. Prefix Logical NOT Operator `!`

Added the `!` (logical NOT) prefix operator as an alternative to `== false`:

```vix
let x = true
if (!x) {
    print("x is false")
}
//is
if (x == false) {
    print("x is false")
}
```

Supports double negation: `!!x` converts any boolean back to itself.

### 2. Unit Type `()`

Added `()` as the unit/void type and value:

```vix
// As a type (equivalent to void):
fn do_nothing(): () { }

// As a value (equivalent to nil):
let x: () = ()
```

### 3. Deprecation Warnings

The compiler now emits warnings for deprecated syntax patterns:

- **`struct NAME {...}` syntax**: Use `type NAME = struct {...}` instead
- **`-> type` return syntax**: Use `: type` instead for function return types

```vix
// No!
struct Foo { x: i32 }
fn bar() -> i32 { return 42 }

// Yes!
type Foo = struct { x: i32 }
fn bar(): i32 { return 42 }
```

## Bug Fixes

### ADT Runtime Fixes

- **Fixed ADT constructors in arrays**: `Some(x)`, `Ok(x)`, `Err(x)`, and custom ADT constructors with payloads now use a proper tagged struct representation `{tag: i32, payload: i8*}`. Previously, `Some(x)` returned the raw value and `None` returned null, causing type mismatches when storing in arrays or matching.

- **Fixed `None` representation**: `None` now creates a tagged struct with `tag=1` and `payload=null`, consistent with `Some(x)` which uses `tag=0`. This ensures type compatibility in all contexts.

- **Fixed `match` for `Some`/`None`**: Match arms for `Some(v)` and `None` now use tag-based comparison (`x.0 == 0` / `x.0 == 1`) instead of nil pointer comparison. Bare `None` patterns in match arms are correctly handled.

- **Fixed `pointerElementHints` propagation**: ADT struct type hints are now properly propagated through variable assignment, enabling correct `.0`/`.1` field access on ADT values stored in variables.

- **Fixed polymorphic ADT constructor sharing**: Added `freshen_type()` to deep-copy types with fresh variables on each constructor use. Previously, `Ok(Socket{...})` and `Ok(0)` would incorrectly share type variables, causing `Socket vs I32` errors.

- **Fixed ADT payload type for `Err`/`Some`**: `check_member` for `Result[T, E].1` now correctly returns `E` (not `T`) when inside an `Err` match arm, by checking `match_payload_field_types` before the default ADT type lookup.

- **Fixed ADT heap allocation**: ADT constructors (`None`, `Some`, `Ok`, `Err`, custom) now heap-allocate the tagged struct instead of stack-allocating. This fixes dangling pointer issues when returning ADT values from functions.

- **Fixed `let x = expr : type` annotation**: Parser rules `LET identifier ASSIGN expression COLON type` now correctly set `declared_type`, so type annotations after the expression are properly applied (e.g., `let a = Ok(42) : Result[i32, string]`).

### Type Checker Fixes

- **Fixed `nil` → `FixedArray` coercion**: `nil` can now be assigned to `[u8 * 8]` and other fixed array fields in struct literals, useful for zero-initialized buffers.

- **Fixed numeric promotion in function call arguments**: `i32` literals now automatically promote to `i64`/`usize` in function call arguments (e.g., `connect(fd, &addr, 16)` where `addrlen` is `usize`).

### Syntax Fixes

- **Fixed `type NAME[T] = struct {...}`**: Generic struct definitions using the `type` keyword now work correctly. Previously, only `type NAME = struct {...}` (non-generic) and `type NAME[T] = ...` (generic enum) were supported.

### Pointer Improvements

- **Improved dereference error messages**: Attempting to dereference a non-pointer type now shows a more helpful message: `"cannot dereference non-pointer type (use '@' only on pointer types)"`.

- **Added pointer mutability check**: Assigning through a dereferenced pointer (`@ptr = value`) now checks if the pointer variable is mutable and reports a clear error if not.

## Standard Library

- what?

## New Tests

Added 24 new tests in `tests/test_types.py` covering:

- **Logical NOT Operator** (4 tests): `!true`, `!false`, `!comparison`, `!!double_not`
- **Generic Struct via `type` keyword** (2 tests): Definition with generics, string fields
- **ADT Constructors** (2 tests): Option match with Some/None, None match
- **Pointer Error Detection** (2 tests): Dereference non-pointer, valid pointer dereference
- **Edge Cases** (6 tests): Nested if, function expression, empty function, chained member access, mutable let, for-loop array iteration
- **ADT Match Payload Types** (5 tests): Err payload type, Ok payload type, both arms, option from function, tag annotation syntax
- **Unit Type** (1 test): Unit value `()` as expression
- **Logical NOT in Context** (2 tests): NOT in while loop, NOT with and/or

**Total: 89 tests passing** (up from 65 in v0.2.1)

## Files Changed

- `include/ast.h`: Added `OP_NOT` to `UnaryOpType`
- `src/parser/lexer.l`: Added `BANG` token for `!`
- `src/parser/parser.y`: Added `!` unary rule, `()` type/expression, deprecation warnings, generic struct `type[T]` syntax, fixed match desugaring for `Some`/`None`, fixed `let x = expr : type` annotation
- `src/Typeck/Typeck.cpp`: Added `OP_NOT` type checking, `freshen_type()` for ADT ctor instantiation, `nil`→`FixedArray` coercion, numeric promotion in function args, improved dereference error messages, pointer mutability check, fixed ADT payload type for `Err` match arms, fixed desugared match payload type detection
- `src/compiler/CodeGen.cpp`: Added `OP_NOT` codegen, fixed ADT tagged struct representation for `Some`/`None`, fixed `pointerElementHints` propagation, added `pointerElementHints` for ADT allocas, heap-allocated ADT structs to fix dangling pointer returns
- `std/net.vix`: Fixed compilation errors
- `tests/test_types.py`: Added 24 new tests (89 total, up from 65)
- `Docs/RELEASE_v0.2.2.md`: This file

## Migration Guide

1. Replace `struct NAME {...}` with `type NAME = struct {...}` (deprecated but still works).
2. Replace `-> type` with `: type` for function return types (deprecated but still works).
3. Use `!expr` instead of `expr == false` for logical negation.
4. For generic structs, use `type NAME[T] = struct {...}` syntax.
