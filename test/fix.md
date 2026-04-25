## 已修复的问题

1. HOF/lambda 调用路径崩溃（`CreateSExtOrTrunc` 断言）
- 文件: `src/compiler/backend-llvm/LlvmEmit.cpp`
- 修复: 在 `castValue` 中给整数到指针转换增加类型保护，非整数不再走 `SExtOrTrunc`，改为安全 `IntCast/BitCast`。

2. 函数值调用类型不稳导致误编译/崩溃
- 文件: `src/compiler/backend-llvm/LlvmEmit.cpp`
- 修复: `emitFunctionPointerCall` 增加返回类型 hint 和参数类型适配，统一处理 `fn` 值调用。

3. lambda 作为表达式污染外层插入点
- 文件: `src/compiler/backend-llvm/LlvmEmit.cpp`
- 修复: `visitFunction` 增加 `saveIP/restoreIP`，避免函数节点代码生成后清空调用方插入点。

4. 嵌套函数捕获外层局部变量触发 invalid IR/segfault
- 文件: `src/compiler/backend-llvm/LlvmEmit.cpp`
- 修复: 在 `visitIdentifier` 中检测跨函数局部读取，改为明确 `SemanticError`（不再崩溃）。

5. struct/list 嵌套索引与 length 结果错误
- 文件: `src/compiler/backend-llvm/LlvmEmit.cpp`
- 修复:
	- 增加 `memberArrayLengthHints/memberNestedArrayLengthHints`，修复 `t.scopes.length` 与 `t.scopes[0].length`。
	- 修正 `pointerElementHints` 传播，修复 `t.scopes[1][0]` 崩溃。

6. 循环引用 struct（按值自递归）崩溃
- 文件: `src/compiler/backend-llvm/LlvmEmit.cpp`
- 修复: `visitStructDef` 对按值自递归字段报 `SemanticError: self-recursive struct fields must use pointer type`，避免 segfault。

## 关键用例结果

- `test7.vix`: 编译通过，运行输出 `42`。
- `test18.vix`: 编译通过，运行输出 `matched-err`。
- `test13.vix`: 输出 `2`（修复前为 `0`）。
- `test14.vix`: 输出 `2`（修复前为 `1`）。
- `test15.vix`: 输出 `3`（修复前运行 segfault）。
- `test16.vix`/`test17.vix`: 不再 segfault，改为明确语义错误。

## 自动化测试脚本

- 已实现 `test/run.py`。
- 支持自动执行 `testN.vix`，并对关键样例校验:
	- 应该编译失败的测试（捕获、循环引用）是否失败且报错文本正确。
	- 应该运行成功的测试（如 `test7/test18`）输出是否匹配。

运行方式:

```bash
cd test
python3 run.py
```

## 说明

- 目前“闭包捕获外层局部变量”仍未实现为可运行语义（当前行为为明确报错），但已从“崩溃/坏 IR”修复为“可诊断错误”。
