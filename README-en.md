![Vix logo](images/README/1770378110202.png)

# Vix Programming Language

[![Self-hosting Progress](https://img.shields.io/badge/Self--hosting-90%25-orange)]()
[![Backends](https://img.shields.io/badge/Backends-LLVM%20%7C%20QBE%20%7C%20C++-brightgreen)]()
[![License](https://img.shields.io/badge/License-Apache%202.0-blue)]()

Vix is a lightweight, statically typed, compiled language. The goal is to keep the syntax simple while providing performance close to native languages.

[中文版](README.md) | [Quick Start](#quick-start) | [Documentation](#documentation) | [VS Code Extension](https://github.com/Daweidie/vix-lang-analyzer) | [Contributing](#contributing)

## Highlights

- Static typing with compile-time checks
- Cross-platform (Windows / Linux / macOS)
- LLVM backend (this repository also contains historical work around QBE/C++ backends)

## Quick Start

### Dependencies

Build dependencies typically include: `clang/llvm`, `flex`, `bison`, `make`, and `git`.

This repository provides a simple dependency install script (Linux):

```bash
./src/install.sh
```

### Build the compiler

```bash
cd src
make
```

After building, the compiler binary is located at `src/vixc`.

### Verify

```bash
./src/vixc -v
```

### Your first program

Create `hello.vix`:

```vix
fn main() -> i32 {
    print("Hello, Vix!")
    return 0
}
```

Compile and run:

```bash
./src/vixc hello.vix -o hello
./hello
```

## Examples

The [examples](examples) and [examples/test](examples/test) directories contain many runnable examples.

Two common snippets:

### Fibonacci

```vix
fn fib(n: i32) -> i32 {
    if (n <= 1) {
        return n
    }
    return fib(n - 1) + fib(n - 2)
}

fn main() -> i32 {
    print(fib(10))
    return 0
}
```

### for loop

```vix
fn main() -> i32 {
    mut sum = 0
    for (i in 1 .. 100) {
        sum = sum + i
    }
    print("sum=", sum)
    return 0
}
```

## Documentation

- [CONTRIBUTING.md](Docs/CONTRIBUTING.md) — How to contribute
- [compiler.md](Docs/compiler.md) — Compiler internals and workflow
- [control-flow.md](Docs/control-flow.md) — Control flow (if/loops)
- [functions.md](Docs/functions.md) — Functions, calls, generics
- [getting-started.md](Docs/getting-started.md) — Install/build and first program
- [modules.md](Docs/modules.md) — Modules, import/export
- [pointers.md](Docs/pointers.md) — Pointers
- [stdlib.md](Docs/stdlib.md) — Standard library
- [structs.md](Docs/structs.md) — Structs
- [syntax.md](Docs/syntax.md) — Syntax reference (incl. EBNF)
- [types.md](Docs/types.md) — Type system
- [what-is-vix.md](Docs/what-is-vix.md) — Overview and design goals

Tip: if you just want to get a working build quickly, start with [getting-started.md](Docs/getting-started.md).

## Contributing

We welcome all forms of contribution: language/syntax proposals, documentation, bug reports, code contributions, and standard library improvements.

Please read the [Contribution Guidelines](Docs/CONTRIBUTING.md) to get started.

## Project Ecosystem

Vix is gradually building its own ecosystem:

| Project            | Description                         | Status            |
| ------------------ | ----------------------------------- | ----------------- |
| **Vix Compiler**   | Core compiler (LLVM/QBE/C++ backends) | In development, self-hosting soon |
| **VPM**            | Vix Package Manager                 | Community contribution |
| **Standard Library** | Common data structures and functions | Community contribution |
| **VS Code Extension** | Editor support                    | Released          |

## License

This project is open-sourced under the Apache License 2.0 - see the [LICENSE](LICENSE) file for details.

## Contact

- Email: [popolk1871@outlook.com](mailto:popolk1871@outlook.com)
- GitHub Issues: Submit directly in this repository

- QQ group: 130577506

**If you're interested in Vix, feel free to star, fork, open an issue, or try it out right away!**
