# Functions

Functions are the fundamental building blocks of a Vix program. This document details the function features of Vix.

## Table of Contents

- [Function Definition](#function-definition)
- [Parameters](#parameters)
- [Return Values](#return-values)
- [Function Calls](#function-calls)
- [Scope](#scope)
- [Recursion](#recursion)
- [Generic Functions](#generic-functions)
- [External Functions](#external-functions)
- [Public Functions](#public-functions)

---

## Function Definition

### Basic Syntax

```vix
fn function_name(parameters) -> return_type {
    // function body
    return value
}
```
Of course, you can also use:
```vix
fn function_name(parameters): return_type {
    // function body
    return value
}
```
Both syntaxes co-exist!

### Examples

```vix
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn greet(name: string) {
    print("Hello, " + name + "!")
}
```

### Main Function

Every Vix program must have a `main` function as its entry point:

```vix
fn main() -> i32 {
    // program code
    return 0
}
```

Main function with command-line arguments:

```vix
fn main(argc: i32, argv: ptr) -> i32 {
    for (i in 0 .. argc) {
        print(argv[i])
    }
    return 0
}
```

---

## Parameters

### Parameter Declaration

Parameters use the `name: type` syntax:

```vix
fn power(base: i32, exponent: i32) -> i32 {
    let result = 1
    for (i in 0 .. exponent) {
        result *= base
    }
    return result
}
```

### Supported Types

All basic types and composite types can be used as parameters:

```vix
// Basic types
fn processInt(n: i32) { }
fn processFloat(f: f64) { }
fn processBool(b: bool) { }
fn processString(s: string) { }

// Pointers
fn processPointer(p: &i32) { }
fn processGenericPointer(p: ptr) { }

// Arrays
fn processArray(arr: [i32], len: i32) { }
fn processFixedSizeArray(arr: [i32 * 5]) { }

// Structs
struct Point { x: f64, y: f64 }
fn processPoint(p: Point) { }
```

### Variadic Parameters

Use `...` to represent variadic parameters:

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
}
```

---

## Return Values

### return Statement

Use the `return` keyword to return a value:

```vix
fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}
```

### No Return Value

Functions can return no value (implicitly returns void):

```vix
fn log(message: string) {
    print("[LOG] " + message)
    // No return statement
}
```

Explicitly declaring void return type:

```vix
fn doSomething() -> void {
    // function body
}
```

### Multiple Return Paths

```vix
fn absolute(n: i32) -> i32 {
    if (n < 0) {
        return -n
    }
    return n
}
```

---

## Function Calls

### Basic Calling

```vix
let result = add(3, 5)
print(result)  // 8
```

### Nested Calling

```vix
let result = add(multiply(2, 3), 4)
print(result)  // 10
```

### Chained Calling

```vix
fn double(n: i32) -> i32 {
    return n * 2
}

let result = double(double(double(2)))
print(result)  // 16
```

---

## Scope

### Local Variables

Variables declared inside a function are local:

```vix
fn example() {
    let x = 10      // x is only valid inside example
    print(x)
}

fn another() {
    // print(x)     // Error: x is undefined
}
```

### Variable Shadowing

Inner scopes can shadow outer variables:

```vix
let x = 10

fn example() {
    let x = 20      // shadows outer x
    print(x)        // 20
}

print(x)            // 10
```

### Block Scope

```vix
fn example() {
    let a = 1
    {
        let b = 2
        print(a)    // 1 - can access outer variable
        print(b)    // 2
    }
    // print(b)     // Error: b is out of scope
    print(a)        // 1
}
```

---

## Recursion

Functions can call themselves:

### Basic Recursion

```vix
fn factorial(n: i32) -> i32 {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

print(factorial(5))  // 120
```

### Fibonacci Sequence

```vix
fn fib(n: i32) -> i32 {
    if (n <= 1) {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}

print(fib(10))  // 55
```

### Tail Recursion Optimization

The Vix compiler optimizes tail recursion:

```vix
fn fibTail(n: i32, a: i32, b: i32) -> i32 {
    if (n == 0) {
        return a
    }
    return fibTail(n - 1, b, a + b)
}

print(fibTail(40, 0, 1))  // Efficient calculation
```

---

## Generic Functions

### Defining Generic Functions

Use the `[T]` syntax to define generics:

```vix
fn identity:[T](value: T) -> T {
    return value
}

fn add:[T](a: T, b: T) -> T {
    return a + b
}
```

### Using Generic Functions

```vix
// Explicitly specify type
let a = identity:[i32](42)
let b = identity:[string]("Hello")
let c = add:[f64](2.5, 3.5)

// Let the compiler infer
let d = identity(42)        // Inferred as i32
let e = identity("Hello")   // Inferred as string
```

### Generic Constraints

Generic parameters must support the required operations:

```vix
// T must support the + operation
fn sum:[T](a: T, b: T, c: T) -> T {
    return a + b + c
}

let result = sum:[i32](1, 2, 3)  // 6
```

---

## External Functions

Use `extern` to declare external functions:

### C Standard Library Functions

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
    fn puts(s: ptr) -> i32
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
    fn memcpy(dest: ptr, src: ptr, n: i32) -> ptr
    fn memset(s: ptr, c: i32, n: i32) -> ptr
}
```

### Using External Functions

```vix
extern "C" {
    fn printf(format: ptr, ...) -> i32
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

fn main() -> i32 {
    // Allocate memory
    let buffer = malloc(100)
    
    // Use memory
    for (i in 0 .. 10) {
        buffer[i] = i * i
    }
    
    // Print results
    for (i in 0 .. 10) {
        printf("%d ", buffer[i])
    }
    printf("\n")
    
    // Free memory
    free(buffer)
    return 0
}
```

### System Calls

```vix
extern "C" {
    fn system(cmd: ptr) -> i32
    fn exit(status: i32) -> void
}

fn main() -> i32 {
    system("echo Hello from shell")
    return 0
}
```

---

## Public Functions

Use the `pub` keyword to export functions:

### Defining Public Functions

```vix
// math.vix
pub fn square(n: i32) -> i32 {
    return n * n
}

pub fn cube(n: i32) -> i32 {
    return n * n * n
}

// Internal function, not exported
fn helper() {
    // ...
}
```

### Importing and Using

```vix
import "math.vix"

fn main() -> i32 {
    let sq = square(5)   // 25
    let cb = cube(3)     // 27
    print(sq)
    print(cb)
    return 0
}
```

---

## Functions as Values

Vix supports function pointers:

```vix
fn apply(a: i32, b: i32, op: fn(i32, i32) -> i32) -> i32 {
    return op(a, b)
}

fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}

fn main() -> i32 {
    print(apply(3, 4, add))       // 7
    print(apply(3, 4, multiply))  // 12
    return 0
}
```

---

## Inline Functions

The compiler automatically inlines small functions to improve performance:

```vix
fn square(n: i32) -> i32 {
    return n * n
}

fn main() -> i32 {
    // square might be inlined
    let result = square(5)
    print(result)
    return 0
}
```

---

## Best Practices

1. **Single Responsibility**: Each function should do only one thing.
2. **Meaningful Naming**: Function names should clearly express their purpose.
3. **Appropriate Length**: Functions should not be too long; no more than 50 lines is recommended.
4. **Minimize Side Effects**: Use pure functions as much as possible.
5. **Explicit Typing**: Use explicit type annotations for public functions.

---

## Examples

### Math Library

```vix
pub fn abs(n: i32) -> i32 {
    if (n < 0) {
        return -n
    }
    return n
}

pub fn max(a: i32, b: i32) -> i32 {
    if (a > b) {
        return a
    }
    return b
}

pub fn min(a: i32, b: i32) -> i32 {
    if (a < b) {
        return a
    }
    return b
}

pub fn clamp(value: i32, low: i32, high: i32) -> i32 {
    return max(low, min(value, high))
}
```

### String Processing Functions

```vix
pub fn strlen(s: string) -> i32 {
    return s.length
}

pub fn strconcat(a: string, b: string) -> string {
    return a + b
}
```

---

## Next Steps

- [Control Flow](control-flow.md) - Control flow statements
- [Structs](structs.md) - Structs and methods
- [Module System](modules.md) - Modules and organizing code