# Type System

Vix employs a static type system with type checking performed at compile time. This document details the Vix type system.

## Table of Contents

- [Basic Types](#basic-types)
- [Integer Types](#integer-types)
- [Floating-point Types](#floating-point-types)
- [Boolean Type](#boolean-type)
- [String Type](#string-type)
- [Pointer Types](#pointer-types)
- [Array Types](#array-types)
- [Struct Types](#struct-types)
- [Generic Types](#generic-types)
- [Type Inference](#type-inference)
- [Type Casting](#type-casting)

---

## Basic Types

Vix provides the following basic types:

| Type | Description | Size | Default Value |
|------|------|------|--------|
| `i8` | 8-bit signed integer | 1 byte | 0 |
| `i32` | 32-bit signed integer | 4 bytes | 0 |
| `i64` | 64-bit signed integer | 8 bytes | 0 |
| `f32` | 32-bit floating-point | 4 bytes | 0.0 |
| `f64` | 64-bit floating-point | 8 bytes | 0.0 |
| `bool` | Boolean type | 1 byte | false |
| `string` | String type | - | "" |
| `void` | Void type | 0 | - |
| `ptr` | Generic pointer | 8 bytes | nil |
| `usize` | Unsigned integer | 8 bytes | 0 |

---

## Integer Types

### i8 - 8-bit Integer

Used for storing small integers:

```vix
let small: i8 = 127    // maximum value
let tiny: i8 = -128    // minimum value
```

Range: -128 to 127

### i32 - 32-bit Integer

The most commonly used integer type:

```vix
let count: i32 = 1000000
let index: i32 = 0
let result: i32 = 42
```

Range: -2,147,483,648 to 2,147,483,647

### i64 - 64-bit Integer

Used for scenarios requiring a larger range:

```vix
let bigNumber: i64 = 9223372036854775807
let timestamp: i64 = 1704067200000
```

Range: -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807

### Integer Literals

```vix
let decimal = 42        // decimal
let hex = 0x2A          // hexadecimal
let binary = 0b101010   // binary
let octal = 0o52        // octal
```

---

## Floating-point Types

### f32 - Single-precision Floating-point

Suitable for scenarios with lower precision requirements:

```vix
let temperature: f32 = 36.5
let ratio: f32 = 0.618
```

Precision: Approx. 6-7 significant digits.

### f64 - Double-precision Floating-point

The default floating-point type with higher precision:

```vix
let pi: f64 = 3.14159265358979
let e: f64 = 2.71828182845904
```

Precision: Approx. 15-16 significant digits.

### Floating-point Literals

```vix
let a = 3.14
let b = 2.0
let c = 1.5e10      // scientific notation
let d = 2.5e-3      // 0.0025
```

---

## Boolean Type

The `bool` type has only two values: `true` and `false`.

```vix
let isActive: bool = true
let isEmpty: bool = false
let result = 10 > 5  // result is of type bool, value is true
```

### Boolean Operations

```vix
let a = true
let b = false

let andResult = a and b    // false
let orResult = a or b      // true
let notResult = !a         // false
```

---

## String Type

The `string` type is used to store text data:

```vix
let greeting: string = "Hello, Vix!"
let name = "World"
let empty = ""
```

### String Operations

```vix
let s = "Hello"

// String length
print(s.length)      // 5

// String concatenation
let full = s + " World"  // "Hello World"

// String comparison
if (strcmp(s, "Hello") == 0) {
    print("Strings are equal")
}
```

### String Literals

```vix
let simple = "Hello"
let withEscape = "Line1\nLine2"
let withTab = "Column1\tColumn2"
let withQuote = "She said \"Hi\""
```

### Escape Characters

| Escape Sequence | Description |
|----------|------|
| `\n` | Newline |
| `\t` | Tab |
| `\r` | Carriage return |
| `\\` | Backslash |
| `\"` | Double quote |
| `\'` | Single quote |

---

## Pointer Types

Pointers store memory addresses and are used for direct memory manipulation.

### Declaring Pointers

```vix
// Pointer to a specific type
let intPtr: &i32
let floatPtr: &f64

// Generic pointer
let genericPtr: ptr
```

### Address-of and Dereference

```vix
let x = 10
mut ptr = &x        // address-of: ptr points to x

let value = @ptr    // dereference: get the value pointed to by ptr
@ptr = 20           // modify x's value through the pointer
```

### Null Pointer

```vix
let nullPtr: &i32 = nil

if (nullPtr == nil) {
    print("Pointer is null")
}
```

### Pointer Arithmetic

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]
let second = @(p + 1)  // get arr[1] = 2
```

---

## Array Types

### Fixed-size Array

```vix
// Syntax: [element_type * size]
let arr: [i32 * 5] = [1, 2, 3, 4, 5]

// Access element
print(arr[0])       // 1

// Modify element
arr[1] = 10

// Get length
print(arr.length)   // 5
```

### Dynamic List

```vix
// Syntax: [element_type]
let list: [i32] = [1, 2, 3]

// Or let the compiler infer
let list2 = [1, 2, 3, 4, 5]
```

### Multi-dimensional Array

```vix
// 2D Array
let matrix: [[i32 * 3] * 3] = [
    [1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]
]

print(matrix[0][0])  // 1
print(matrix[1][2])  // 6
```

---

## Struct Types

A struct is a user-defined composite type.

### Defining a Struct

```vix
struct Person {
    name: string,
    age: i32,
    height: f64
}
```

### Creating an Instance

```vix
let p = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}
```

### Accessing Fields

```vix
print(p.name)    // "Alice"
print(p.age)     // 25
print(p.height)  // 5.7
```

### Nested Structs

```vix
struct Address {
    street: string,
    city: string,
    zip: string
}

struct Person {
    name: string,
    address: Address
}

let p = Person {
    name: "Bob",
    address: Address {
        street: "123 Main St",
        city: "Anytown",
        zip: "12345"
    }
}

print(p.address.city)  // "Anytown"
```

---

## Generic Types

Vix supports generics, allowing for the creation of parameterized types and functions.

### Generic Structs

```vix
struct Box:[T] {
    value: T
}

// Instantiation
let intBox = Box:[i32]{ value: 42 }
let floatBox = Box:[f64]{ value: 3.14 }
let strBox = Box:[string]{ value: "Hello" }
```

### Generic Functions

```vix
fn identity:[T](x: T) -> T {
    return x
}

fn add:[T](a: T, b: T) -> T {
    return a + b
}

// Usage
let a = identity:[i32](42)
let b = identity:[string]("Hello")
let c = add:[f64](2.5, 3.5)
```

### Sum Types with Type Constraints

```vix
type Flag:[T] = On | Off

let state: Flag:[bool] = On

match state {
    On -> {
        print("Flag is on")
    }
    Off -> {
        print("Flag is off")
    }
}
```

---

## Type Inference

Vix supports type inference, where the compiler infers the type of a variable based on the context.

### Literal Inference

```vix
let a = 42        // inferred as i32
let b = 3.14      // inferred as f64
let c = "Hello"   // inferred as string
let d = true      // inferred as bool
```

### Expression Inference

```vix
let x = 10        // i32
let y = 20        // i32
let z = x + y     // i32

let f1 = 1.5      // f64
let f2 = f1 * 2   // f64
```

### Return Value Inference

```vix
fn getValue() -> i32 {
    return 42
}

let result = getValue()  // result inferred as i32
```

---

## Type Casting

### Numeric Type Conversion

Conversions between numeric types may be automatic or require explicit annotation:

```vix
let a: i32 = 10
let b: i64 = a       // automatic promotion
let c: f64 = a       // integer to float
```

### Pointer Type Casting

```vix
let ptr: ptr = malloc(100)
let intPtr: &i32 = ptr  // generic pointer to specific type pointer
```

---

## New Type Syntax (2026-03)

### Optional Types

`?T` represents a type that can be a null value (implemented via pointer semantics in the current implementation):

```vix
fn lookup:[T](table: SymbolTable:[T], name: string): ?T {
    return None
}
```

### Function Types

Support for passing functions as first-class values:

```vix
fn map:[T,U](list: [T], f: fn(T): U): [U] {
    ...
}
```

### Union Types

Support for union type definitions with generic parameters:

```vix
type Result:[T,E] = Ok(T) | Err(E)
```

Can be used with `match`:

```vix
let ok = Ok(42) : Result[i32, string]
match ok {
    Ok(v) -> print(v)
    Err(e) -> print(e)
}
```

### Generic Type Parameter Syntax

The declaration side supports `:[T]`; the usage side supports `:[Type]`:

```vix
struct SymbolTable:[T] { ... }
let tab = new_table:[Type]()
```

---

## Type Compatibility

### Assignment Compatibility

```vix
let a: i32 = 10
let b: i64 = a    // i32 can be assigned to i64

let x: f32 = 1.5
let y: f64 = x    // f32 can be assigned to f64
```

### Function Parameter Compatibility

```vix
fn process(n: i64) {
    print(n)
}

let x: i32 = 42
process(x)  // i32 can be passed to an i64 parameter
```

---

## Best Practices

1. **Prefer Type Inference**: Let the compiler infer types for simple variables.
2. **Use Explicit Annotations for Complex Types**: Use explicit types for function parameters, return values, etc.
3. **Choose Appropriate Integer Size**: Select `i8`, `i32`, or `i64` based on the range of values.
4. **Default to f64**: Use `f64` as the default floating-point type unless there are specific needs.
5. **Use Generics for Code Reuse**: Use generics for universal data structures.

---

## Next Steps

- [Functions in Detail](functions.md) - Learn about the relationship between functions and types
- [Structs](structs.md) - Deep dive into struct types