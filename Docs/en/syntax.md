# Vix Language Syntax Reference

This document provides a comprehensive syntax reference for the Vix programming language.

## Table of Contents

- [Program Structure](#program-structure)
- [Comments](#comments)
- [Keywords](#keywords)
- [Identifiers](#identifiers)
- [Variables](#variables)
- [Data Types](#data-types)
- [Operators](#operators)
- [Control Flow](#control-flow)
- [Functions](#functions)
- [Structs](#structs)
- [Arrays and Lists](#arrays-and-lists)
- [Pointers](#pointers)
- [Module System](#module-system)
- [Error Handling](#error-handling)

---

## Program Structure

### Entry Point

Every Vix program starts execution from the `main` function:

```vix
fn main(): i32 {
    // program code
    return 0
}
```

Entry point with command-line arguments:

```vix
fn main(argc: i32, argv: ptr): i32 {
    for (i in 0 .. argc) {
        print(argv[i])
    }
    return 0
}
```

### Program Components

A Vix program can contain:

- Global variable declarations
- Struct definitions
- Function definitions
- Import statements
- External function declarations

```vix
// Import modules
import "std/io.vix"

// Global variable
global counter = 42

// Struct definition
struct Point {
    x: f64,
    y: f64
}

// External function declaration
extern "C" {
    fn printf(format: ptr, ...) -> i32
}

// Function definition
fn main(): i32 {
    print("Hello, Vix!")
    return 0
}
```

---

## Comments

### Single-line Comments

Use `//` to start a comment that ends at the end of the line:

```vix
// This is a single-line comment
let x = 10  // comment at the end of a line
```

### Multi-line Comments

Use `/* */` to wrap comments:

```vix
/*
 * This is a
 * multi-line comment
 */
let x = 10
```

---

## Keywords

Reserved keywords in Vix:

| Keyword | Usage |
|--------|------|
| `fn` | Function definition |
| `let` | Immutable variable declaration |
| `mut` | Mutable variable declaration |
| `if` | Conditional check |
| `elif` | Else-if |
| `else` | Else |
| `while` | while loop |
| `for` | for loop |
| `in` | loop range/iterator |
| `return` | return statement |
| `struct` | struct definition |
| `import` | module import |
| `pub` | public declaration |
| `extern` | external function declaration |
| `global` | global variable declaration |
| `nil` | null pointer |
| `and` | logical AND |
| `or` | logical OR |
| `match` | pattern matching |

---

## Identifiers

Identifiers are used to name variables, functions, structs, etc.

### Naming Rules

- Must start with a letter or an underscore
- Can contain letters, numbers, and underscores
- Case-sensitive

```vix
// Valid identifiers
myVariable
_myVar
MyFunction
value1

// Invalid identifiers
1value      // cannot start with a digit
my-var      // cannot contain hyphens
```

### Naming Conventions

- **Variables**: lowerCamelCase (`myVariable`)
- **Functions**: lowerCamelCase (`calculateSum`)
- **Structs**: PascalCase (`MyStruct`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_SIZE`)

---

## Variables

### Variable Declaration

#### Variables

Use the `let` keyword to declare a variable:

```vix
let x = 10
let name = "Vix"
let pi = 3.14159
```

### Type Annotation

You can explicitly specify the type during declaration:

```vix
let a: i32 = 10
let b: f64 = 3.14
let c: string = "Hello"
let flag: bool = true
```

### Variable Assignment

```vix
mut x = 10
x = 20        // simple assignment
x += 5        // composite assignment
x -= 3
x *= 2
x /= 4
```

---

## Data Types

### Basic Types

| Type | Description | Size |
|------|------|------|
| `i8` | 8-bit signed integer | 1 byte |
| `i32` | 32-bit signed integer | 4 bytes |
| `i64` | 64-bit signed integer | 8 bytes |
| `f32` | 32-bit floating-point | 4 bytes |
| `f64` | 64-bit floating-point | 8 bytes |
| `bool` | boolean type | 1 byte |
| `string` | string type | - |
| `void` | void type | - |
| `ptr` | generic pointer type | - |
| `usize` | unsigned integer (pointer size) | - |

### Type Annotation Syntax

```vix
let num: i32 = 42
let float: f64 = 3.14
let text: string = "Hello"
let flag: bool = true
let ptr: &i32 = &num // pointer type
let arr: [i32 * 5] = [1, 2, 3, 4, 5] // array
```

---

## Operators

### Arithmetic Operators

| Operator | Description | Example |
|--------|------|------|
| `+` | Addition | `a + b` |
| `-` | Subtraction | `a - b` |
| `*` | Multiplication | `a * b` |
| `/` | Division | `a / b` |
| `%` | Modulo | `a % b` |
| `**` | Exponentiation | `a ** b` |

### Comparison Operators

| Operator | Description | Example |
|--------|------|------|
| `==` | Equal | `a == b` |
| `!=` | Not Equal | `a != b` |
| `<` | Less than | `a < b` |
| `<=` | Less than or equal | `a <= b` |
| `>` | Greater than | `a > b` |
| `>=` | Greater than or equal | `a >= b` |

### Logical Operators

| Operator | Description | Example |
|--------|------|------|
| `and` | Logical AND | `a and b` |
| `or` | Logical OR | `a or b` |
| `!` | Logical NOT | `!a` |

### Bitwise Operators

| Operator | Description | Example |
|--------|------|------|
| `&` | Bitwise AND | `a & b` |
| `|` | Bitwise OR | `a | b` |
| `^` | Bitwise XOR | `a ^ b` |
| `~` | Bitwise NOT | `~a` |
| `<<` | Left shift | `a << n` |
| `>>` | Right shift | `a >> n` |

### Pointer Operators

| Operator | Description | Example |
|--------|------|------|
| `&` | Address-of | `&x` |
| `@` | Dereference | `@ptr` |

### Operator Precedence

From highest to lowest:

1. `()` `[]` `.` `@`
2. `!` `~` `-` (unary) `&`
3. `**`
4. `*` `/` `%`
5. `+` `-`
6. `<<` `>>`
7. `<` `<=` `>` `>=`
8. `==` `!=`
9. `&` (bitwise)
10. `^`
11. `|`
12. `and`
13. `or`
14. `=` `+=` `-=` `*=` `/=`

---

## Control Flow

### if Statement

```vix
if (condition) {
    // code block
}

// with elif and else
if (x > 10) {
    print("x is greater than 10")
} elif (x > 5) {
    print("x is greater than 5")
} else {
    print("x is 5 or less")
}
```

### while Loop

```vix
let i = 0
while (i < 10) {
    print(i)
    i += 1
}
```

### for Loop

#### Range Loop

```vix
// From 1 up to 9 (excluding 10)
for (i in 1 .. 10) {
    print(i)
}
```

#### Nested Loops

```vix
for (x in 1 .. 3) {
    for (y in 1 .. 3) {
        print("x:", x, "y:", y)
    }
}
```

### break and continue

```vix
// break - exit the loop
let i = 0
while (true) {
    if (i >= 10) {
        break
    }
    print(i)
    i += 1
}

// continue - skip current iteration
for (i in 1 .. 10) {
    if (i % 2 == 0) {
        continue
    }
    print(i)  // only prints odd numbers
}
```

### match Statement

```vix
match value {
    10 -> {
        print("value is 10")
    }
    20 -> {
        print("value is 20")
    }
    _ -> {
        print("value is something else")
    }
}
```

---

## Functions

### Function Definition

```vix
fn function_name(param1: type1, param2: type2): return_type {
    // function body
    return value
}
```

### Basic Examples

```vix
fn add(a: i32, b: i32): i32 {
    return a + b
}

fn greet(name: string) {
    print("Hello, " + name + "!")
}
```

### Public Functions

Use the `pub` keyword to declare an exportable function:

```vix
pub fn publicFunction(x: i32): i32 {
    return x * 2
}
```

### External Functions

```vix
extern "C" {
    fn printf(format: ptr, ...): i32
    fn malloc(size: i32): ptr
    fn free(ptr: ptr): void
}
```

### Generic Functions

```vix
fn id:[T](value: T): T {
    return value
}

fn add:[T](a: T, b: T): T {
    return a + b
}

// usage
let x = id:[i32](42)
let y = add:[f64](2.5, 3.5)
```

---

## Structs

### Defining Structs

```vix
struct Person {
    name: string,
    age: i32,
    height: f64
}
```

### Creating Instances

```vix
// using initializer list
let p1 = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

// field-by-field assignment
let p2: Person {}
p2.name = "Bob"
p2.age = 30
p2.height = 6.0
```

### Accessing Fields

```vix
print(p1.name)    // "Alice"
print(p1.age)     // 25
print(p1.height)  // 5.7
```

### Generic Structs

```vix
struct Box:[T] {
    value: T
}

let intBox = Box:[i32]{ value: 42 }
let floatBox = Box:[f64]{ value: 3.14 }
```

---

## Arrays and Lists

### Fixed-size Arrays

```vix
// Declaration
let arr: [i32 * 5] = [1, 2, 3, 4, 5]

// Access elements
print(arr[0])      // 1
print(arr[2])      // 3

// Modify elements
arr[1] = 10

// Array length
print(arr.length)  // 5
```

### Dynamic Lists

```vix
fn map:[T, U](list: [T], f: fn(T): U): [U]
{
    let out = []
    for (item in list)
    {
        out.push!(f(item))
    }
    return out
}

fn main(): i32
{
    let nums = [1, 2, 3]
    let doubled = map:[i32, i32](nums, fn(x: i32): i32 { x * 2 })
    print(doubled[0])
    print(doubled[1])
    print(doubled[2])
    return 0
}
```

### Arrays as Function Parameters

```vix
fn printArray(arr: [i32], len: i32) {
    for (i in 0 .. len) {
        print(arr[i])
    }
}
```

---

## Pointers

### Address-of and Dereference

```vix
let x = 10
let mut ptr = &x        // get the address of x

let value = @ptr    // dereference: get the value pointed to by ptr
@ptr = 20           // modify the value through the pointer
```

### Pointer Arithmetic

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]
let second = @(p + 1)  // get arr[1] = 2
print(second)
```

### Null Pointers

```vix
let p: &i32 = nil

if (p == nil) {
    print("Pointer is null")
}
```

---

## Module System

### Importing Modules

```vix
// import standard library modules
import "std/io.vix"
import "std/arr.vix"
import "std/strings.vix"
```

### Exporting Functions

Use the `pub` keyword to mark functions that need to be exported:

```vix
// mymodule.vix
pub fn myPublicFunction(x: i32): i32 {
    return x * 2
}

fn privateHelper() {
    // this function will not be exported
}
```

### Using Imported Functions

```vix
import "mymodule.vix"

fn main(): i32 {
    let result = myPublicFunction(5)
    print(result)
    return 0
}
```

---

## Error Handling

---

## Syntax Extensions (2026-03)

The following syntax has been verified through the compilation pipeline (`vixc -> out.ll -> clang -> run`) in the `src/test.vix` scenario.

### Generic Declarations and Calls

Supports two generic styles (while maintaining compatibility):

```vix
fn new_table:[T](): SymbolTable:[T] { ... }
let tab = new_table[Type]()

struct SymbolTable:[T] {
    scopes: [ [ (string, T) ] ]
}
```

### match and Constructor Patterns

Supports constructor patterns and binding in `match` branches:

```vix
match t {
    Some(v) -> print(v.name)
    None -> print("not found")
}

match ok {
    Ok(v) -> print(v)
    Err(e) -> print(e)
}
```

### Union Types (ADT)

Supports union type definitions:

```vix
type Result:[T, E] = Ok(T) | Err(E)
```

### Other Compatible Syntax

- Optional types: `?T`
- Function types: `fn(T): U`
- Tuple-style arguments: `(a, b)` (e.g., `push((name, val))`)
- Tuple-style index fields: `pair.0`, `pair.1`
- `for (item in iterable)` form
- Line comments: `// comment`

### Current Implementation Notes

- Generic function calls will fallback to default backend instantiation if type parameters are not explicitly provided.
- Union types and constructors are currently usable; complex scenarios (deep nesting, full ADT optimization) are continuously evolving.

---

## Input/Output

### Output

```vix
// Print string
print("Hello, World!")

// Print multiple values
print("Name:", name, "Age:", age)

// Formatted output
printf("Value: %d, Text: %s\n", 42, "test")
```
Notes: `printf` requires `extern "C" { fn printf(fmt: ptr, ...): i32 }`.

---

## Global Variables

Use the `global` keyword to declare a global variable:

```vix
global counter = 42
global message: string = "Hello World"
global pi: f64 = 3.14159

fn main() -> i32 {
    print(counter)
    print(message)
    print(pi)
    return 0
}
```

---

## Next Steps

- [Type System](types.md) - Deep dive into the type system
- [Functions in Detail](functions.md) - More function features
- [Standard Library](stdlib.md) - Built-in functionality reference