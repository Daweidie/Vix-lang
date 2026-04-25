# Module System

The Vix module system allows you to organize your code into reusable modules. Through the `import` and `pub` keywords, you can easily share and reuse code.

## Table of Contents

- [Importing Modules](#importing-modules)
- [Exporting Functions](#exporting-functions)
- [Standard Library Modules](#standard-library-modules)
- [Creating Custom Modules](#creating-custom-modules)
- [Module Organization](#module-organization)
- [Best Practices](#best-practices)

---

## Importing Modules

### Basic Syntax

Use the `import` keyword to import a module:

```vix
import "path/to/module.vix"
```

### Importing the Standard Library

```vix
import "std/io.vix"
import "std/arr.vix"
import "std/strings.vix"
import "std/os.vix"
import "std/mem.vix"
import "std/rand.vix"
```

### Using Imported Functions

```vix
import "std/io.vix"
import "std/strings.vix"

fn main() -> i32 {
    let s = "Hello, Vix!"
    
    // Use the imported function
    puts(s)
    
    let cmp = strcmp("abc", "def")
    print(cmp)
    
    return 0
}
```

---

## Exporting Functions

### pub Keyword

Use the `pub` keyword to mark functions that need to be exported:

```vix
// math_utils.vix

// Public function - can be imported and used by other modules
pub fn square(n: i32) -> i32 {
    return n * n
}

pub fn cube(n: i32) -> i32 {
    return n * n * n
}

pub fn power(base: i32, exp: i32) -> i32 {
    let result = 1
    for (i in 0 .. exp) {
        result *= base
    }
    return result
}

// Private function - only used within this module
fn helper(x: i32) -> i32 {
    return x * 2
}
```

### Importing Custom Modules

```vix
// main.vix
import "math_utils.vix"

fn main() -> i32 {
    let sq = square(5)    // 25
    let cb = cube(3)      // 27
    let pw = power(2, 10) // 1024
    
    print(sq)
    print(cb)
    print(pw)
    
    // helper(10)  // Error: helper is not a public function
    
    return 0
}
```

---

## Standard Library Modules

### std/io - Input/Output

```vix
import "std/io.vix"

fn main() -> i32 {
    // Output
    puts("Hello, World!")
    
    // File operations
    let file = fopen("test.txt", "w")
    if (file == nil) {
        puts("Failed to open file")
        return 1
    }
    
    let data = "Hello, Vix!"
    fwrite(data, 1, data.length, file)
    fclose(file)
    
    return 0
}
```

**Available Functions:**
| Function | Description |
|------|------|
| `puts(s)` | Print string and a newline |
| `fopen(filename, mode)` | Open a file |
| `fclose(file)` | Close a file |
| `fread(ptr, size, count, file)` | Read from a file |
| `fwrite(ptr, size, count, file)` | Write to a file |
| `panic(msg)` | Trigger an error and stop the program |

### std/arr - Array Operations

```vix
import "std/arr.vix"

fn main() -> i32 {
    let arr = [5, 2, 8, 1, 9, 10]
    
    // Sort the array
    sort(arr, arr.length)
    
    // Print the sorted array
    for (i in 0 .. arr.length) {
        printf("%d ", arr[i])
    }
    printf("\n")
    
    return 0
}
```

**Available Functions:**
| Function | Description |
|------|------|
| `sort(arr, size)` | Bubble sort an array |

### std/strings - String Operations

```vix
import "std/strings.vix"

fn main() -> i32 {
    let str1 = "Hello"
    let str2 = "World"
    
    // String comparison
    let cmp = strcmp(str1, str2)
    if (cmp == 0) {
        print("Strings are equal")
    } elif (cmp < 0) {
        print("str1 < str2")
    } else {
        print("str1 > str2")
    }
    
    return 0
}
```

**Available Functions:**
| Function | Description |
|------|------|
| `strcmp(a, b)` | Compare two strings |

### std/os - Operating System Interface

```vix
import "std/os.vix"

fn main() -> i32 {
    // Execute system command
    system("echo Hello from shell")
    
    // Directory operations
    let dir = opendir(".")
    // ... read directory
    closedir(dir)
    
    return 0
}
```

**Available Functions:**
| Function | Description |
|------|------|
| `system(cmd)` | Execute a system command |
| `exit(status)` | Exit the program |
| `opendir(path)` | Open a directory |
| `readdir(dir)` | Read a directory entry |
| `closedir(dir)` | Close a directory |

### std/mem - Memory Management

```vix
import "std/mem.vix"

fn main() -> i32 {
    // Allocate memory
    let buffer = malloc(100)
    
    // Set memory
    memset(buffer, 0, 100)
    
    // Use memory...
    buffer[0] = 42
    
    // Free memory
    free(buffer)
    
    return 0
}
```

**Available Functions:**
| Function | Description |
|------|------|
| `malloc(size)` | Allocate memory |
| `free(ptr)` | Free memory |
| `memcpy(dest, src, n)` | Copy memory |
| `memset(s, c, n)` | Set memory |
| `memcmp(s1, s2, n)` | Compare memory |

### std/rand - Random Numbers

```vix
import "std/rand.vix"

fn main() -> i32 {
    // Generate a random number between 1 and 100
    let r = rand(1, 100)
    print(r)
    
    return 0
}
```

**Available Functions:**
| Function | Description |
|------|------|
| `rand(min, max)` | Generate a random number in the specified range |

### std/net - Network Programming

```vix
import "std/net/net.vix"

fn main() -> i32 {
    // Create socket
    let sock = socket(2, 1, 0)
    
    if (sock < 0) {
        printf("socket failed\n")
        return 1
    }
    
    // Listen for connections
    if (listen(sock, 5) < 0) {
        printf("listen failed\n")
        return 1
    }
    
    printf("server running\n")
    
    // Process client
    let response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Hello from Vix!</h1>"
    
    while (1) {
        let client = accept(sock, 0, 0)
        if (client >= 0) {
            printf("client connected\n")
            send(client, response, 100, 0)
            close(client)
        }
    }
    
    close(sock)
    return 0
}
```

---

## Creating Custom Modules

### Module File Structure

```
myproject/
├── main.vix
├── utils.vix
└── math/
    ├── basic.vix
    └── advanced.vix
```

### Creating Utility Module

```vix
// utils.vix

pub fn clamp(value: i32, min: i32, max: i32) -> i32 {
    if (value < min) {
        return min
    }
    if (value > max) {
        return max
    }
    return value
}

pub fn abs(n: i32) -> i32 {
    if (n < 0) {
        return -n
    }
    return n
}

pub fn sign(n: i32) -> i32 {
    if (n > 0) {
        return 1
    }
    if (n < 0) {
        return -1
    }
    return 0
}
```

### Creating Math Module

```vix
// math/basic.vix

pub fn add(a: i32, b: i32) -> i32 {
    return a + b
}

pub fn subtract(a: i32, b: i32) -> i32 {
    return a - b
}

pub fn multiply(a: i32, b: i32) -> i32 {
    return a * b
}

pub fn divide(a: i32, b: i32) -> i32 {
    if (b == 0) {
        return 0
    }
    return a / b
}
```

```vix
// math/advanced.vix

pub fn power(base: i32, exp: i32) -> i32 {
    let result = 1
    for (i in 0 .. exp) {
        result *= base
    }
    return result
}

pub fn factorial(n: i32) -> i32 {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

pub fn fibonacci(n: i32) -> i32 {
    if (n <= 1) {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}
```

### Using Custom Modules

```vix
// main.vix
import "utils.vix"
import "math/basic.vix"
import "math/advanced.vix"

fn main() -> i32 {
    // Use utils
    let clamped = clamp(150, 0, 100)
    print(clamped)  // 100
    
    // Use math/basic
    let sum = add(10, 20)
    print(sum)      // 30
    
    // Use math/advanced
    let fact = factorial(5)
    print(fact)     // 120
    
    return 0
}
```

---

## Module Organization

### Recommended Project Structure

```
project/
├── src/
│   ├── main.vix       # Main program entry
│   ├── app.vix        # Application logic
│   └── modules/
│       ├── config.vix # Configuration module
│       ├── utils.vix  # Utility functions
│       └── types.vix  # Type definitions
├── lib/
│   └── mylib.vix      # External library
└── std/
    ├── io.vix
    └── strings.vix
```

### Module Naming Convention

- Use lowercase letters
- Use underscores to separate words
- Names should be meaningful and concise

```
string_utils.vix   # Good
StringUtils.vix    # Not recommended
su.vix             # Too short, not clear
```

---

## Best Practices

### 1. Single Responsibility

Each module should be responsible for only one functional area:

```vix
// Good: focuses on string operations
// string_utils.vix
pub fn trim(s: string) -> string { }
pub fn split(s: string, delim: string) -> [string] { }
pub fn join(arr: [string], delim: string) -> string { }

// Bad: mixes many unrelated features
// utils.vix
pub fn trim(s: string) { }
pub fn sortArray(arr: [i32]) { }
pub fn connectToDatabase() { }
```

### 2. Clear Public Interface

Only export necessary functions:

```vix
// math.vix

// Public interface
pub fn calculate(x: i32, y: i32) -> i32 {
    return internalHelper(x) + internalHelper(y)
}

// Private implementation details
fn internalHelper(n: i32) -> i32 {
    return n * n + 1
}
```

### 3. Avoid Circular Dependencies

```
// Error: circular dependency
// a.vix
import "b.vix"

// b.vix
import "a.vix"  // Circular!

// Correct: refactor code, extract common part
// common.vix
pub fn sharedFunction() { }

// a.vix
import "common.vix"

// b.vix
import "common.vix"
```

### 4. Reasonable Import Order

```vix
// Standard library
import "std/io.vix"
import "std/strings.vix"

// Third-party libraries
import "lib/http.vix"

// Local modules
import "utils.vix"
import "config.vix"
```

---

## Next Steps

- [Standard Library](stdlib.md) - Detailed standard library documentation
- [Functions](functions.md) - Function definition and exporting
- [Compiler](compiler.md) - Compiling multi-file projects