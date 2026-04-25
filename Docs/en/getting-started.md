# Getting Started

This guide will help you quickly get started with the Vix programming language. We'll start from installation and guide you through your first Vix program step by step.

## Installing Vix

### Compiling from Source

#### System Requirements

- **Operating System**: Windows, Linux, or macOS
- **Dependencies**:
  - LLVM (Version 14+ recommended)
  - Make build tool
  - C Compiler (GCC or Clang)

#### Compilation Steps

1. **Clone the Repository**

```shell
git clone https://github.com/vix-lang/vix.git
cd vix
```

2. **Enter the Source Directory and Compile**

```shell
cd src
make
```

3. **Verify Installation**

```shell
vixc -v
```

If you see version information, the installation was successful!

### Installation Location

After compilation, the `vixc` executable is located in the `src` directory. You can add it to your system PATH for global access:

```shell
# Linux/macOS
export PATH=$PATH:/path/to/vix/src

# Windows (PowerShell)
$env:Path += ";C:\path\to\vix\src"
```

## First Program

### Hello World

Create a file named `hello.vix`:

```vix
fn main() -> i32 {
    print("Hello, Vix!")
    return 0
}
```

### Compile and Run

```shell
# Compile
vixc hello.vix -o hello

# Run
./hello        # Linux/macOS
hello.exe      # Windows
```

Output:
```
Hello, Vix!
```

## Quick Syntax Overview

### Variable Declaration

```vix
// Immutable variable (using let)
let a = "Hello"
let b = 123
let c = 3.14

// Mutable variable (using mut)
mut x = 10
x = 20  // Can be modified

// With type annotation
let a: i32 = 10
let b: f64 = 3.14
let c: string = "Hello"
```

### Function Definition

```vix
// Basic function
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

// Main function
fn main() -> i32 {
    let result = add(3, 5)
    print(result)
    return 0
}
```

### Control Flow

```vix
// if statement
if (x > 10) {
    print("x is greater than 10")
} elif (x > 5) {
    print("x is greater than 5")
} else {
    print("x is 5 or less")
}

// while loop
let i = 0
while (i < 5) {
    print(i)
    i += 1
}

// for loop
for (i in 1 .. 10) {
    print(i)
}
```

### Array Operations

```vix
// Fixed-size array
let arr: [i32 * 5] = [1, 2, 3, 4, 5]
print(arr[0])       // 1
print(arr.length)   // 5

// Dynamic list
let list = [1, 2, 3]
list.push!(4)       // Add element
let last = list.pop() // Pop last element
```

### Structs

```vix
struct Person {
    name: string,
    age: i32
}

fn main() -> i32 {
    let p = Person { name: "Alice", age: 25 }
    print(p.name)
    print(p.age)
    return 0
}
```

## Compiler Options

### Common Commands

```shell
# Compile to executable
vixc source.vix -o output

# Specify LLVM backend
vixc source.vix -o output --backend=llvm

# Generate LLVM IR
vixc source.vix -ll output_ir

# Enable optimization
vixc source.vix -o output -opt
```

### Initialize Project

```shell
vixc init
```

This will create a basic project structure and configuration files.

## Next Steps

Now that you have the basics of Vix, you can continue learning:

- [Language Syntax Reference](syntax.md) - Complete syntax documentation
- [Type System](types.md) - Learn about Vix's type system
- [Functions in Detail](functions.md) - Deep dive into function features
- [Standard Library](stdlib.md) - Explore built-in functionality

## Self-hosting Compiler

Vix is moving towards self-hosting! `vixc0` is a Vix compiler written in Vix:

```shell
# Compile the self-hosting compiler
cd vixc0
vixc main.vix -o vixc0

# Use vixc0 to compile code
./vixc0 --emit-ll hello.vix hello.ll
```

Learn more: [What is Vix](what-is-vix.md#self-hosting-compiler-vixc0)

## Example Programs

### Fibonacci Sequence

```vix
fn fib(n: i32) -> i64 {
    let a = 0
    let b = 1
    for (i in 1 .. n) {
        let c = a + b
        a = b
        b = c
    }
    return b
}

fn main() -> i32 {
    print(fib(40))
    return 0
}
```

### Command-line Arguments

```vix
fn main(argc: i32, argv: ptr) -> i32 {
    for (i in 0 .. argc) {
        print(argv[i])
    }
    return 0
}
```

### File Operations

```vix
import "std/io.vix"

fn main() -> i32 {
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

## Common Issues

### LLVM Not Found During Compilation

Ensure LLVM is installed and correct environment variables are set:

```shell
# Linux
export LLVM_HOME=/usr/lib/llvm-14

# macOS
export LLVM_HOME=/usr/local/opt/llvm
```

### Shared Library Not Found at Runtime

If you encounter issues loading runtime libraries, ensure library paths are added to the system search path:

```shell
# Linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/libs

# macOS
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:/path/to/libs
```

## Getting Help

- **Documentation**: Refer to other documents on this site
- **Examples**: Check the `examples` directory of the project
- **Community**: Join the QQ group 130577506
- **Issues**: Submit issues on GitHub

---

> Ready? Let's start writing Vix programs!