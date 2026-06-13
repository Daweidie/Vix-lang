<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="images/README/image.png">
    <source media="(prefers-color-scheme: light)" srcset="images/README/image.png">
    <img
      alt="Vix Programming Language"
      src="images/README/image.png"
      width="50%">
  </picture>

[Website][Vix] | [Getting Started][Getting Started] | [Learn][Learn] | [Documentation][Documentation] | [Contributing][Contributing] 
</div>
[![zread](https://img.shields.io/badge/Ask_Zread-_.svg?style=for-the-badge&color=00b0aa&labelColor=000000&logo=data%3Aimage%2Fsvg%2Bxml%3Bbase64%2CPHN2ZyB3aWR0aD0iMTYiIGhlaWdodD0iMTYiIHZpZXdCb3g9IjAgMCAxNiAxNiIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj4KPHBhdGggZD0iTTQuOTYxNTYgMS42MDAxSDIuMjQxNTZDMS44ODgxIDEuNjAwMSAxLjYwMTU2IDEuODg2NjQgMS42MDE1NiAyLjI0MDFWNC45NjAxQzEuNjAxNTYgNS4zMTM1NiAxLjg4ODEgNS42MDAxIDIuMjQxNTYgNS42MDAxSDQuOTYxNTZDNS4zMTUwMiA1LjYwMDEgNS42MDE1NiA1LjMxMzU2IDUuNjAxNTYgNC45NjAxVjIuMjQwMUM1LjYwMTU2IDEuODg2NjQgNS4zMTUwMiAxLjYwMDEgNC45NjE1NiAxLjYwMDFaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00Ljk2MTU2IDEwLjM5OTlIMi4yNDE1NkMxLjg4ODEgMTAuMzk5OSAxLjYwMTU2IDEwLjY4NjQgMS42MDE1NiAxMS4wMzk5VjEzLjc1OTlDMS42MDE1NiAxNC4xMTM0IDEuODg4MSAxNC4zOTk5IDIuMjQxNTYgMTQuMzk5OUg0Ljk2MTU2QzUuMzE1MDIgMTQuMzk5OSA1LjYwMTU2IDE0LjExMzQgNS42MDE1NiAxMy43NTk5VjExLjAzOTlDNS42MDE1NiAxMC42ODY0IDUuMzE1MDIgMTAuMzk5OSA0Ljk2MTU2IDEwLjM5OTlaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik0xMy43NTg0IDEuNjAwMUgxMS4wMzg0QzEwLjY4NSAxLjYwMDEgMTAuMzk4NCAxLjg4NjY0IDEwLjM5ODQgMi4yNDAxVjQuOTYwMUMxMC4zOTg0IDUuMzEzNTYgMTAuNjg1IDUuNjAwMSAxMS4wMzg0IDUuNjAwMUgxMy43NTg0QzE0LjExMTkgNS42MDAxIDE0LjM5ODQgNS4zMTM1NiAxNC4zOTg0IDQuOTYwMVYyLjI0MDFDMTQuMzk4NCAxLjg4NjY0IDE0LjExMTkgMS42MDAxIDEzLjc1ODQgMS42MDAxWiIgZmlsbD0iI2ZmZiIvPgo8cGF0aCBkPSJNNCAxMkwxMiA0TDQgMTJaIiBmaWxsPSIjZmZmIi8%2BCjxwYXRoIGQ9Ik00IDEyTDEyIDQiIHN0cm9rZT0iI2ZmZiIgc3Ryb2tlLXdpZHRoPSIxLjUiIHN0cm9rZS1saW5lY2FwPSJyb3VuZCIvPgo8L3N2Zz4K&logoColor=ffffff)](https://zread.ai/vixlang/Vix-lang)
This is the main source repository for [Vix]. It contains the compiler,
runtime-related components, examples, and language documentation.

[Vix]: https://vixlang.github.io
[Getting Started]: Docs/en/getting-started.md
[Learn]: Docs/en/what-is-vix.md
[Documentation]: Docs/en/syntax.md
[Contributing]: https://github.com/vixlang/Vix-lang/issues
[Chinese README]: README-zh_CN.md

## Why Vix?

- **Performance:** Vix compiles to native code with an LLVM-based backend and is designed for low-overhead execution.

- **Reliability:** Static typing and compile-time checks catch common errors earlier.

- **Simplicity:** The language keeps syntax concise while still supporting practical features like functions, modules, structs, pointers, generics, and control flow.

## Hello world!

``` vix 
import "std/io.vix"
fn main(): i32
{
    puts("Hello,world!")
    return 0
}
```

## Repository Layout

- `src/`: Compiler source code and build scripts.
- `include/`: Public/internal headers for parser, type system, codegen, and semantic analysis.
- `examples/`: Language examples and sample programs.
- `docs/` : RELEASE NOTES.
- `test/`: Language regression and behavior tests.
- `CMakeLists.txt`: Top-level CMake entry for project builds.

## Documentation

- LearnVix:[GitHub Link](https://github.com/vixlang/LearnVix)
- VixDocs:[GitHub Link](https://github.com/vixlang/LearnVix)
- VixDocs for ZRead:[Link](https://zread.ai/vixlang/Vix-lang)

## Getting Help

- Open a discussion in [GitHub Issues](https://github.com/Daweidie/vix-lang/issues).
- Contact: [popolk1871@outlook.com](mailto:popolk1871@outlook.com)
- QQ Group: 130577506

## Contributing

Contributions are welcome, including language design feedback, bug reports,
tests, standard library improvements, and documentation updates.

To start contributing, please open or pick an issue:
[vix-lang issues](https://github.com/Daweidie/vix-lang/issues)

## License

Vix is distributed under the Apache License 2.0.
See [LICENSE](LICENSE) for details.

## Ecosystem

| Project               | Description                                            | Status                                                           |
| -----------------------| --------------------------------------------------------| ------------------------------------------------------------------|
| **Vix Compiler**      | Core compiler with LLVM-focused backend implementation | In active development                                            |
| **Very**              | Package manager for Vix                                | Community contribution [Very](https://github.com/vixlang/Very)   |
| **Standard Library**  | Common APIs and utilities                              | Community contribution                                           |
| **VS Code Extension** | Editor support for Vix                                 | Published [Link](https://github.com/vixlang/ext-VixLangAnalyzer) |