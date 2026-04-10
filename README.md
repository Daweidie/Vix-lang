![Vix logo](images/README/1770378110202.png)

# Vix 编程语言

[![自举进度](https://img.shields.io/badge/自举-90%25-orange)]()
[![后端](https://img.shields.io/badge/后端-LLVM%20%7C%20QBE%20%7C%20C++-brightgreen)]()
[![许可证](https://img.shields.io/badge/许可证-Apache%202.0-blue)]()

Vix 是一种轻量级、静态类型的编译型语言，目标是在保持语法简洁的同时，提供接近原生语言的执行效率。

[English](README-en.md) | [快速开始](#快速开始) | [文档](#文档) | [VS Code 扩展](https://github.com/Daweidie/vix-lang-analyzer) | [参与贡献](#参与贡献)

## 特性概览

- 静态类型 + 编译期检查
- 多平台（Windows / Linux / macOS）
- LLVM 后端（仓库内历史上也包含过 QBE/C++ 相关实现）

## 快速开始

### 依赖

构建依赖主要包括：`clang/llvm`、`flex`、`bison`、`make`、`git`。

仓库提供了一个简单的依赖安装脚本（Linux）：

```bash
./src/install.sh
```

### 构建编译器

```bash
cd src
make
```

编译完成后，编译器二进制位于 `src/vixc`。

### 验证

```bash
./src/vixc -v
```

### 第一个程序

创建 `hello.vix`：

```vix
fn main() -> i32 {
    print("Hello, Vix!")
    return 0
}
```

编译并运行：

```bash
./src/vixc hello.vix -o hello
./hello
```

## 示例代码

仓库的 [examples](examples) 和 [examples/test](examples/test) 目录中包含大量可运行示例。

这里给出两段最常见的语法片段：

### 斐波那契

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

### for 循环

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

##  文档

- [CONTRIBUTING.md](Docs/CONTRIBUTING.md) —— 贡献指南，如何参与项目开发
- [compiler.md](Docs/compiler.md) —— 编译器实现细节与工作流程
- [control-flow.md](Docs/control-flow.md) —— 控制流语句（if、循环等）的语法与用法
- [functions.md](Docs/functions.md) —— 函数定义、调用、泛型等特性
- [getting-started.md](Docs/getting-started.md) —— 快速入门指南，安装与第一个程序
- [modules.md](Docs/modules.md) —— 模块系统，导入与导出规则
- [pointers.md](Docs/pointers.md) —— 指针的声明、解引用、运算及使用示例
- [stdlib.md](Docs/stdlib.md) —— 标准库提供的函数与常用模块
- [structs.md](Docs/structs.md) —— 结构体定义、实例化、字段访问
- [syntax.md](Docs/syntax.md) —— 完整语法参考，包含 EBNF 形式
- [types.md](Docs/types.md) —— 类型系统：基本类型、泛型、联合类型等
- [what-is-vix.md](Docs/what-is-vix.md) —— Vix 语言简介与设计目标

> 提示：如果你只想从零开始跑通一次编译 + 运行，建议先看 [getting-started.md](Docs/getting-started.md)。

## 参与贡献

我们欢迎各种形式的贡献！包括但不限于：提出语法建议、撰写文档、报告 bug、提交代码、完善标准库等。

请阅读[贡献指南](Docs/CONTRIBUTING.md)开始。

## 项目生态

Vix 正在逐步构建自己的生态：


| 项目             | 描述                            | 状态             |
| ---------------- | ------------------------------- | ---------------- |
| **Vix 编译器**   | 核心编译器（LLVM/QBE/C++ 后端） | 开发中，即将自举 |
| **VPM**          | Vix 包管理器                    | 社区贡献中       |
| **标准库**       | 常用数据结构和函数              | 社区贡献中       |
| **VS Code 扩展** | 编辑器支持                      | 已发布           |

## 许可证

本项目基于 Apache License 2.0 开源 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系方式

- 邮箱：[popolk1871@outlook.com](mailto:popolk1871@outlook.com)
- GitHub Issues：直接在本仓库提交

- QQ 群：130577506（一起聊 Vix）

**如果你对 Vix 感兴趣，欢迎 star、fork、提 issue，或者直接上手试试！**

