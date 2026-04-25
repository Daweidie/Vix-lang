# Standard Library

Vix provides a standard library containing commonly used data structures and functions. This document details how to use the standard library.

## Table of Contents

- [std/io - Input/Output](#stdio---inputoutput)
- [std/arr - Array Operations](#stdarr---array-operations)
- [std/strings - String Operations](#stdstrings---string-operations)
- [std/os - Operating System Interface](#stdos---operating-system-interface)
- [std/mem - Memory Management](#stdmem---memory-management)
- [std/rand - Random Numbers](#stdrand---random-numbers)
- [std/net - Network Programming](#stdnet---network-programming)

---

## std/io - Input/Output

Provides basic input/output and file operation functionality.

### Importing

```vix
import "std/io.vix"
```

### Function List

| Function | Signature | Description |
|------|------|------|
| `puts` | `fn puts(s: ptr) -> i32` | Print string and a newline |
| `fopen` | `fn fopen(filename: ptr, mode: ptr) -> ptr` | Open a file |
| `fclose` | `fn fclose(file: ptr) -> i32` | Close a file |
| `fread` | `fn fread(ptr: ptr, size: usize, count: usize, file: ptr) -> usize` | Read from a file |
| `fwrite` | `fn fwrite(ptr: ptr, size: usize, count: usize, file: ptr) -> usize` | Write to a file |
| `panic` | `fn panic(msg: ptr) -> i32` | Print error message and suspend the program |

### Usage Examples

#### Console Output

```vix
import "std/io.vix"

fn main() -> i32 {
    puts("Hello, Vix!")
    return 0
}
```

#### File Writing

```vix
import "std/io.vix"

fn main() -> i32 {
    let file = fopen("output.txt", "w")
    if (file == nil) {
        puts("Failed to open file for writing")
        return 1
    }
    
    let data = "Hello, this is written by Vix!"
    fwrite(data, 1, data.length, file)
    fclose(file)
    
    puts("File written successfully")
    return 0
}
```

#### File Reading

```vix
import "std/io.vix"

extern "C" {
    fn malloc(size: i32) -> ptr
    fn free(ptr: ptr) -> void
}

fn main() -> i32 {
    let file = fopen("input.txt", "r")
    if (file == nil) {
        puts("Failed to open file for reading")
        return 1
    }
    
    // Allocate buffer
    let buffer = malloc(1024)
    let bytesRead = fread(buffer, 1, 1024, file)
    
    // Process data...
    printf("Read %d bytes\n", bytesRead)
    
    fclose(file)
    free(buffer)
    return 0
}
```

#### Error Handling

```vix
import "std/io.vix"

fn main() -> i32 {
    let file = fopen("nonexistent.txt", "r")
    if (file == nil) {
        panic("Cannot open required file")
    }
    
    // This part will not be executed
    fclose(file)
    return 0
}
```

---

## std/arr - Array Operations

Provides array sorting and manipulation functionality.

### Importing

```vix
import "std/arr.vix"
```

### Function List

| Function | Signature | Description |
|------|------|------|
| `sort` | `fn sort(nums: [i32], size: i32)` | Bubble sort an array |

### Usage Examples

#### Array Sorting

```vix
import "std/arr.vix"

fn main() -> i32 {
    let arr = [64, 34, 25, 12, 22, 11, 90]
    
    puts("Before sorting:")
    for (i in 0 .. arr.length) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    sort(arr, arr.length)
    
    puts("After sorting:")
    for (i in 0 .. arr.length) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    return 0
}
```

Output:
```
Before sorting:
64 34 25 12 22 11 90 
After sorting:
11 12 22 25 34 64 90 
```

---

## std/strings - String Operations

Provides string comparison and processing functionality.

### Importing

```vix
import "std/strings.vix"
```

### Function List

| Function | Signature | Description |
|------|------|------|
| `strcmp` | `fn strcmp(a: ptr, b: ptr) -> i32` | Compare two strings |

### Usage Examples

#### String Comparison

```vix
import "std/strings.vix"

fn main() -> i32 {
    let str1 = "apple"
    let str2 = "banana"
    let str3 = "apple"
    
    // Comparing different strings
    let cmp1 = strcmp(str1, str2)
    if (cmp1 < 0) {
        printf("%s < %s\n", str1, str2)  // apple < banana
    }
    
    // Comparing same strings
    let cmp2 = strcmp(str1, str3)
    if (cmp2 == 0) {
        printf("%s == %s\n", str1, str3)  // apple == apple
    }
    
    return 0
}
```

#### Parsing Command-line Arguments

```vix
import "std/strings.vix"

fn main(argc: i32, argv: ptr) -> i32 {
    for (i in 1 .. argc) {
        if (strcmp(argv[i], "--help") == 0) {
            puts("Usage: program [options]")
            puts("Options:")
            puts("  --help    Show this help message")
            puts("  --version Show version")
            return 0
        }
        if (strcmp(argv[i], "--version") == 0) {
            puts("Version 1.0.0")
            return 0
        }
    }
    
    puts("Running program...")
    return 0
}
```

---

## std/os - Operating System Interface

Provides functionality for interacting with the operating system.

### Importing

```vix
import "std/os.vix"
```

### Function List

| Function | Signature | Description |
|------|------|------|
| `system` | `fn system(cmd: ptr) -> i32` | Execute a system command |
| `exit` | `fn exit(status: i32) -> void` | Exit the program |
| `opendir` | `fn opendir(path: ptr) -> ptr` | Open a directory |
| `readdir` | `fn readdir(dir: ptr) -> ptr` | Read a directory entry |
| `closedir` | `fn closedir(dir: ptr) -> i32` | Close a directory |

### Usage Examples

#### Executing System Commands

```vix
import "std/os.vix"

fn main() -> i32 {
    // List the current directory
    system("ls -la")
    
    // Create a directory
    system("mkdir -p output")
    
    // Copy a file
    system("cp input.txt output/")
    
    return 0
}
```

#### Exiting the Program

```vix
import "std/os.vix"

fn main() -> i32 {
    let success = doSomething()
    
    if (!success) {
        puts("Operation failed")
        exit(1)
    }
    
    puts("Success!")
    exit(0)
}

fn doSomething() -> bool {
    // Perform operation
    return true
}
```

#### Directory Operations

```vix
import "std/os.vix"

fn main() -> i32 {
    let dir = opendir(".")
    if (dir == nil) {
        puts("Failed to open directory")
        return 1
    }
    
    // Read directory contents
    let entry = readdir(dir)
    while (entry != nil) {
        // Process directory entry
        entry = readdir(dir)
    }
    
    closedir(dir)
    return 0
}
```

---

## std/mem - Memory Management

Provides dynamic memory allocation and management functionality.

### Importing

```vix
import "std/mem.vix"
```

### Function List

| Function | Signature | Description |
|------|------|------|
| `malloc` | `fn malloc(size: i32) -> ptr` | Allocate memory |
| `free` | `fn free(ptr: ptr)` | Free memory |
| `memcpy` | `fn memcpy(dest: ptr, src: ptr, n: i32) -> ptr` | Copy memory |
| `memset` | `fn memset(s: ptr, c: i32, n: i32) -> ptr` | Set memory |
| `memcmp` | `fn memcmp(s1: ptr, s2: ptr, n: i32) -> i32` | Compare memory |

### Usage Examples

#### Dynamic Array

```vix
import "std/mem.vix"

fn main() -> i32 {
    // Allocate space for 10 integers
    let arr = malloc(40)  // 10 * 4 bytes
    
    // Initialize array
    for (i in 0 .. 10) {
        arr[i] = i * i
    }
    
    // Print array
    for (i in 0 .. 10) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    // Free memory
    free(arr)
    return 0
}
```

#### Memory Copying

```vix
import "std/mem.vix"

fn main() -> i32 {
    let src = malloc(100)
    let dst = malloc(100)
    
    // Initialize source data
    for (i in 0 .. 25) {
        src[i] = i
    }
    
    // Copy data
    memcpy(dst, src, 100)
    
    // Verify copying
    for (i in 0 .. 25) {
        printf("%d ", dst[i])
    }
    printf("\n")
    
    free(src)
    free(dst)
    return 0
}
```

#### Zeroing Memory

```vix
import "std/mem.vix"

fn main() -> i32 {
    let buffer = malloc(1024)
    
    // Zero out the buffer
    memset(buffer, 0, 1024)
    
    // Use the buffer...
    
    free(buffer)
    return 0
}
```

---

## std/rand - Random Numbers

Provides random number generation functionality.

### Importing

```vix
import "std/rand.vix"
```

### Function List

| Function | Signature | Description |
|------|------|------|
| `rand` | `fn rand(mini: i32, maxi: i32) -> i32` | Generate a random number within a specified range |

### Usage Examples

#### Generating Random Numbers

```vix
import "std/rand.vix"

fn main() -> i32 {
    // Generate a random number between 1 and 100
    let r1 = rand(1, 100)
    printf("Random number (1-100): %d\n", r1)
    
    // Generate a random digit between 0 and 9
    let r2 = rand(0, 9)
    printf("Random digit: %d\n", r2)
    
    // Simulate a dice roll
    let dice = rand(1, 6)
    printf("Dice roll: %d\n", dice)
    
    return 0
}
```

#### Guessing Game

```vix
import "std/rand.vix"

fn main() -> i32 {
    let target = rand(1, 100)
    let guesses = 0
    
    puts("Guess a number between 1 and 100!")
    
    while (true) {
        let guess = toint(input("Your guess: "))
        guesses += 1
        
        if (guess == target) {
            printf("Correct! You got it in %d guesses!\n", guesses)
            return 0
        } elif (guess < target) {
            puts("Too low!")
        } else {
            puts("Too high!")
        }
    }
}
```

---

## std/net - Network Programming

Provides network programming functionality.

### Importing

```vix
import "std/net/net.vix"
```

### Function List

| Function | Description |
|------|------|
| `socket(domain, type, protocol)` | Create a socket |
| `listen(sock, backlog)` | Listen for connections |
| `accept(sock, addr, addrlen)` | Accept a connection |
| `send(sock, buf, len, flags)` | Send data |
| `close(sock)` | Close a socket |

### Usage Examples

#### HTTP Server

```vix
import "std/net/net.vix"

fn main() -> i32 {
    // Create TCP socket
    let sock = socket(2, 1, 0)  // AF_INET, SOCK_STREAM
    if (sock < 0) {
        printf("Failed to create socket\n")
        return 1
    }
    
    // Start listening
    if (listen(sock, 5) < 0) {
        printf("Failed to listen\n")
        return 1
    }
    
    printf("Server running on port 80...\n")
    
    // HTTP response
    let response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hello from Vix!</h1>"
    
    // Handle requests
    while (1) {
        let client = accept(sock, 0, 0)
        if (client >= 0) {
            printf("Client connected\n")
            send(client, response, 100, 0)
            close(client)
        }
    }
    
    close(sock)
    return 0
}
```

---

## Built-in Functions

In addition to standard library modules, Vix provides several built-in functions that can be used without importing.

### print

Print to console:

```vix
print("Hello, World!")
print("Value:", 42)
print("Name:", name, "Age:", age)
```

### input

Read user input:

```vix
let name = input("Enter your name: ")
print("Hello, " + name)
```

### toint

Convert string to integer:

```vix
let num = toint("123")
print(num + 1)  // 124
```

### tofloat

Convert string to floating-point number:

```vix
let f = tofloat("3.14")
print(f * 2)  // 6.28
```

---

## Next Steps

- [Module System](modules.md) - How to create and use modules
- [Pointers](pointers.md) - Memory operations
- [Compiler](compiler.md) - Compiler options