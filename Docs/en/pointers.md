# Pointers

Pointers are a feature in Vix for direct memory manipulation. Through pointers, you can access and modify the data stored at specific memory addresses.

## Table of Contents

- [Pointer Basics](#pointer-basics)
- [Address-of and Dereference](#address-of-and-dereference)
- [Pointer Types](#pointer-types)
- [Pointer Arithmetic](#pointer-arithmetic)
- [Null Pointers](#null-pointers)
- [Pointers and Arrays](#pointers-and-arrays)
- [Pointers and Functions](#pointers-and-functions)
- [Memory Management](#memory-management)
- [Safety](#safety)

---

## Pointer Basics

A pointer is a variable that stores a memory address. With pointers, you can:

- Get the memory address of a variable
- Access and modify data at an address
- Efficiently pass large amounts of data
- Perform dynamic memory allocation

### Basic Syntax

```vix
// Declare pointer variables
let ptr: &i32      // Pointer to an i32
let fptr: &f64     // Pointer to an f64
let gptr: ptr      // Generic pointer
```

---

## Address-of and Dereference

### Address-of Operator `&`

Use `&` to get the memory address of a variable:

```vix
let x = 10
mut ptr = &x        // ptr now points to the address of x

print(ptr)          // Prints the address value
```

### Dereference Operator `@`

Use `@` to get the value pointed to by a pointer:

```vix
let x = 10
mut ptr = &x

let value = @ptr    // value = 10
print(value)        // 10
```

### Modifying Value via Pointer

```vix
let x = 10
mut ptr = &x

@ptr = 20           // Modify x's value through the pointer
print(x)            // 20
```

### Full Example

```vix
fn main() -> i32 {
    let x = 10
    
    // Get address
    mut ptr = &x
    print("x =", x)           // x = 10
    print("*ptr =", @ptr)     // *ptr = 10
    
    // Modify through pointer
    @ptr = 20
    print("x =", x)           // x = 20
    print("*ptr =", @ptr)     // *ptr = 20
    
    return 0
}
```

---

## Pointer Types

### Typed Pointers

Vix supports pointers to specific types:

```vix
let intPtr: &i32
let floatPtr: &f64
let strPtr: &string
```

### Generic Pointers

The `ptr` type is a generic pointer that can point to any type:

```vix
let genericPtr: ptr

let x = 10
genericPtr = &x
```

### Pointer to Pointer

```vix
let x = 10
mut ptr = &x
mut ptrToPtr = &ptr

print(@(@ptrToPtr))  // 10
```

---

## Pointer Arithmetic

### Pointer Arithmetic

Pointers can be added or subtracted:

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]

// Pointer addition
let second = @(p + 1)    // arr[1] = 2
let third = @(p + 2)     // arr[2] = 3

print(second)            // 2
print(third)             // 3
```

### Iterating Over an Array

```vix
let arr = [10, 20, 30, 40, 50]
let p = &arr[0]

for (i in 0 .. 5) {
    print(@(p + i))
}
```

### Pointers and Offsets

```vix
let data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
let base = &data[0]

// Skip the first 5 elements
let offset = base + 5
print(@offset)           // 6

// Access more elements
print(@(offset + 1))     // 7
print(@(offset + 2))     // 8
```

---

## Null Pointers

### nil Keyword

`nil` represents a null pointer, which does not point to any valid address:

```vix
let ptr: &i32 = nil

if (ptr == nil) {
    print("Pointer is null")
}
```

### Checking for Null Pointers

```vix
fn safeAccess(ptr: &i32) -> i32 {
    if (ptr == nil) {
        print("Error: null pointer")
        return 0
    }
    return @ptr
}

let validPtr: &i32 = nil
let result = safeAccess(validPtr)  // Error: null pointer
```

### Initializing Pointers

```vix
let ptr: &i32 = nil  // Initialize to null

let x = 10
ptr = &x             // Assign it later

print(@ptr)          // 10
```

---

## Pointers and Arrays

### Array Name as a Pointer

An array name can act as a pointer to its first element:

```vix
let arr = [1, 2, 3, 4, 5]
let p = &arr[0]

// p points to the first element of the array
print(@p)            // 1
```

### Iterating Over an Array with Pointers

```vix
fn printArray(arr: [i32], len: i32) {
    let p = &arr[0]
    for (i in 0 .. len) {
        printf("%d ", @(p + i))
    }
    printf("\n")
}
```

### Modifying Array Elements

```vix
fn doubleElements(arr: [i32], len: i32) {
    let p = &arr[0]
    for (i in 0 .. len) {
        @(p + i) = @(p + i) * 2
    }
}

let arr = [1, 2, 3, 4, 5]
doubleElements(arr, 5)
// arr = [2, 4, 6, 8, 10]
```

---

## Pointers and Functions

### Passing Pointer Parameters

Pointers allow a function to modify variables outside its scope:

```vix
fn swap(a: &i32, b: &i32) {
    let temp = @a
    @a = @b
    @b = temp
}

mut x = 10
mut y = 20
swap(&x, &y)
print("x =", x, "y =", y)  // x = 20, y = 10
```

### Returning Pointers

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
}

fn createInt(value: i32) -> &i32 {
    let ptr = malloc(4)
    @ptr = value
    return ptr
}

let p = createInt(42)
print(@p)  // 42
```

### Function Pointers

```vix
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}

fn calculate(a: i32, b: i32, op: fn(i32, i32) -> i32) -> i32 {
    return op(a, b)
}

let result1 = calculate(3, 4, add)       // 7
let result2 = calculate(3, 4, multiply)  // 12
```

---

## Memory Management

### Dynamic Memory Allocation

Use external functions for memory management:

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
    fn realloc(ptr: ptr, size: i32) -> ptr
}
```

### Allocating and Using Memory

```vix
extern "C" {
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
    
    // Read data
    for (i in 0 .. 10) {
        printf("%d ", buffer[i])
    }
    printf("\n")
    
    // Free memory
    free(buffer)
    return 0
}
```

### Memory Copying

```vix
extern "C" {
    fn memcpy(dest: ptr, src: ptr, n: i32) -> ptr
    fn memset(s: ptr, c: i32, n: i32) -> ptr
}

fn clearBuffer(buf: ptr, size: i32) {
    memset(buf, 0, size)
}
```

---

## Safety

### Avoiding Dangling Pointers

```vix
// Dangerous: returning a pointer to a local variable
fn badFunction() -> &i32 {
    let local = 10
    return &local  // Error! local becomes invalid after the function returns
}

// Safe: using dynamic allocation
extern "C" {
    fn malloc(size: i32) -> ptr
}

fn goodFunction() -> &i32 {
    let ptr = malloc(4)
    @ptr = 10
    return ptr
}
```

### Checking Pointer Validity

```vix
fn safeRead(ptr: &i32) -> i32 {
    if (ptr == nil) {
        print("Warning: null pointer access")
        return 0
    }
    return @ptr
}
```

### Preventing Memory Leaks

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

fn example() -> i32 {
    let buffer = malloc(1000)
    
    // Use buffer...
    
    // Ensure it's freed
    free(buffer)
    return 0
}
```

---

## Best Practices

1. **Always Initialize Pointers**: Point to a valid address or set to `nil`.
2. **Check for Null Pointers**: Check if a pointer is `nil` before dereferencing.
3. **Pair Allocation and Free**: Every `malloc` should have a corresponding `free`.
4. **Avoid Dangling Pointers**: Set the pointer to `nil` after freeing it.
5. **Prefer Pass-by-Value**: Unless you need to modify the original value or for performance sensitivity.

---

## Examples

### Linked List Implementation

```vix
extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

struct Node {
    value: i32,
    next: &Node
}

fn createNode(value: i32) -> &Node {
    let node = malloc(16)  // Assume node size is 16 bytes
    @node.value = value
    @node.next = nil
    return node
}

fn append(head: &Node, value: i32) -> &Node {
    let newNode = createNode(value)
    let current = head
    
    while (@current.next != nil) {
        current = @current.next
    }
    @current.next = newNode
    return head
}

fn printList(head: &Node) {
    let current = head
    while (current != nil) {
        printf("%d -> ", @current.value)
        current = @current.next
    }
    printf("nil\n")
}
```

---

## Next Steps

- [Functions](functions.md) - Function parameter passing
- [Structs](structs.md) - Structs and pointers
- [Standard Library](stdlib.md) - Memory management functions