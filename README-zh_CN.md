<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="images/README/image.png">
    <source media="(prefers-color-scheme: light)" srcset="images/README/image.png">
    <img
      alt="Vix 编程语言"
      src="images/README/image.png"
      width="50%">
  </picture>

[官网][Vix] | [快速开始][Getting Started] | [学习][Learn] | [文档][Documentation] | [参与贡献][Contributing] | [English][English README]
</div>

这是 [Vix] 的主源码仓库，包含编译器、运行时相关组件、示例程序以及语言文档。

[Vix]: https://github.com/vixlang/Vix-lang
[Getting Started]: Docs/zh_CN/getting-started.md
[Learn]: Docs/zh_CN/what-is-vix.md
[Documentation]: Docs/zh_CN/syntax.md
[Contributing]: https://github.com/vixlang/Vix-lang/issues
[English README]: README.md

## 为什么选择 Vix？

- **性能：** Vix 以原生代码生成为目标，当前以 LLVM 后端为主，适合对执行效率敏感的场景。

- **可靠性：** 静态类型与编译期检查可以尽早暴露常见错误。

- **简洁性：** 在保持语法简洁的同时，提供函数、模块、结构体、指针、泛型与控制流等实用能力。

## 仓库结构

- `src/`：编译器源码与构建脚本。
- `include/`：解析器、类型系统、语义分析、代码生成等头文件。
- `examples/`：语言示例与样例程序。
- `Docs/en/` 与 `Docs/zh_CN/`：中英文文档。
- `test/`：语言回归测试与行为测试。
- `CMakeLists.txt`：项目顶层 CMake 入口。

## 文档

- [快速入门](Docs/zh_CN/getting-started.md)
- [语法](Docs/zh_CN/syntax.md)
- [类型系统](Docs/zh_CN/types.md)
- [函数](Docs/zh_CN/functions.md)
- [模块](Docs/zh_CN/modules.md)
- [结构体](Docs/zh_CN/structs.md)
- [指针](Docs/zh_CN/pointers.md)
- [控制流](Docs/zh_CN/control-flow.md)
- [标准库](Docs/zh_CN/stdlib.md)

## 获取帮助

- 通过 [GitHub Issues](https://github.com/Daweidie/vix-lang/issues) 提问或反馈。
- 联系邮箱：[popolk1871@outlook.com](mailto:popolk1871@outlook.com)

## 参与贡献

欢迎各种形式的贡献，包括语法建议、缺陷修复、测试补充、标准库完善和文档改进。

你可以从这里开始：
[vix-lang issues](https://github.com/Daweidie/vix-lang/issues)

## 许可证

Vix 基于 Apache License 2.0 发布。
详见 [LICENSE](LICENSE)。

## 项目生态

| 项目 | 描述 | 状态 |
| --- | --- | --- |
| **Vix 编译器** | 核心编译器，当前以 LLVM 后端实现为主 | 持续开发中 |
| **VPM** | Vix 包管理器 | 社区贡献中 |
| **标准库** | 常用 API 与工具集 | 社区贡献中 |
| **VS Code 扩展** | Vix 编辑器支持 | 已发布 |
