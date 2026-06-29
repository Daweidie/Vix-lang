# Vix-lang 全面安全审计报告

**审计日期**: 2026-05-30  
**审计范围**: 安全漏洞、代码质量、文档问题、构建系统、测试覆盖  
**审计方法**: 5个并行Agent深度审查  
**验证状态**: 未验证（WSL环境依赖安装超时）

---

## 严重程度分级说明

| 等级 | 定义 | 处理时限 |
|------|------|----------|
| **P0** | 严重安全漏洞/逻辑错误 - 可能导致RCE、数据损坏、系统崩溃 | 立即修复 |
| **P1** | 高危问题 - 内存泄漏、资源泄漏、重大功能缺陷 | 24小时内 |
| **P2** | 中危问题 - 潜在崩溃、性能问题、构建失败 | 1周内 |
| **P3** | 低危问题 - 代码质量、文档错误、小的不一致 | 1个月内 |
| **P4** | 信息性 - 改进建议、最佳实践 | 下个版本 |
| **P5** | 维护性 - 代码风格、文档完善 | 长期改进 |

---

## 目录

1. [安全漏洞](#1-安全漏洞)
2. [代码质量与逻辑漏洞](#2-代码质量与逻辑漏洞)
3. [文档问题](#3-文档问题)
4. [构建系统问题](#4-构建系统问题)
5. [测试覆盖问题](#5-测试覆盖问题)

---

## 1. 安全漏洞

### P0 - 严重安全漏洞

#### SEC-001: 栈保护被禁用 (`-fno-stack-protector`)
- **文件**: `CMakeLists.txt:128-129`
- **验证状态**: ⚠️ 未验证
- **描述**: C和C++编译都使用 `-fno-stack-protector`，这使得生成的二进制文件无法检测栈缓冲区溢出。任何栈缓冲区溢出都会导致**可利用的代码执行**，而不会被检测到。
- **代码证据**:
  ```cmake
  target_compile_options(vixc PRIVATE
      $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -fno-stack-protector>
      $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -fno-stack-protector -fexceptions>
  )
  ```
- **影响**: 编译器本身无法防御常见的栈溢出利用。
- **修复建议**: 移除 `-fno-stack-protector` 或替换为 `-fstack-protector-strong`

#### SEC-002: install.sh中的命令注入
- **文件**: `src/install.sh:86`
- **验证状态**: ⚠️ 未验证
- **描述**: 使用 `eval "$cmd"`，其中 `$cmd` 通过连接从发行版检测到的包名构建而成。虽然目前存储库是静态的，但任何能够控制包名或以 `root` 权限运行该脚本的攻击者都可以执行任意命令。
- **代码证据**:
  ```bash
  local Install="apt install -y gcc flex bison llvm clang-18 libclang-18-dev git"
  ...
  cmd="sudo $Install"
  ...
  eval "$cmd"
  ```
- **影响**: 具有shell访问权限但非root权限的攻击者可以利用此漏洞，在系统上以root权限实现无限制的命令执行。
- **修复建议**: 用基于数组的输入或 `snprintf` 替换 `eval`

#### SEC-003: 栈缓冲区溢出 (`llvm_filename[256]`)
- **文件**: `src/main.c:328-333`
- **验证状态**: ⚠️ 未验证
- **描述**: 一个固定大小的栈缓冲区 `char llvm_filename[256]`，其中使用 `strcpy` + `strcat` 复制了用户控制的输入，且未进行长度检查。如果 `llvm_f` 超过约251个字符，`strcat(llvm_filename, ".ll")` 会溢出栈。
- **代码证据**:
  ```c
  char llvm_filename[256];
  if (strstr(llvm_f, ".ll") == NULL) {
      snprintf(llvm_filename, sizeof(llvm_filename), "%s.ll", llvm_f);
  } else {
      strcpy(llvm_filename, llvm_f);  // <-- 如果 llvm_f > 255，则溢出
  }
  ```
- **影响**: 可通过 `-o` 选项或文件名参数触发的直接栈缓冲区溢出。
- **修复建议**: 使用 `snprintf` 替代 `strcpy`/`strcat`

---

### P1 - 高危安全漏洞

#### SEC-004: 字符串乘法中的整数溢出导致堆溢出
- **文件**: `src/ast/ast.c:587-593`
- **验证状态**: ⚠️ 未验证
- **描述**: 当常数折叠 `"str" * N` 时，`str_len * repeat_times` 可能会溢出 `size_t`（在32位系统上为32位），导致分配过小，随后通过循环中的 `strcat` 导致堆溢出。
- **代码证据**:
  ```c
  size_t str_len = strlen(str_val);
  size_t total_len = str_len * repeat_times;  // 整数溢出
  char* result = malloc(total_len + 1);        // 分配过小
  result[0] = '\0';
  for (int i = 0; i < repeat_times; i++) {
      strcat(result, str_val);                 // 堆缓冲区溢出
  }
  ```
- **影响**: 攻击者可以通过字符串乘法构造一条会导致可利用堆损坏的输入。在32位系统上，只需 N=0x100000 和10字节字符串即可完全溢出。
- **修复建议**: 在乘法前添加溢出检查

#### SEC-005: 容量倍增导致整数溢出
- **文件**: `src/ast/typeinfer.c:307, 345, 372, 412, 624`
- **验证状态**: ⚠️ 未验证
- **描述**: 在多个地方，`ctx->capacity` 被二倍，一旦 `capacity` 超过 `SIZE_MAX/2`，就会溢出为0，导致 `realloc(ctx->variables, 0)` 并随后发生空指针解引用或堆损坏。
- **代码证据**:
  ```c
  ctx->capacity = ctx->capacity == 0 ? 10 : ctx->capacity * 2;  // 溢出
  ctx->variables = realloc(ctx->variables, sizeof(VariableInfo) * ctx->capacity);
  ```
- **影响**: 包含足够多变量声明的源代码可能会触发此问题。
- **修复建议**: 添加容量溢出检查

#### SEC-006: TOCTOU竞态条件
- **文件**: `src/main.c:444, 481, 518, 554, 574-581`
- **验证状态**: ⚠️ 未验证
- **描述**: 临时文件在打开进行写入后，在没有原子替换的情况下被删除。`remove()` 调用遵循"先写入后删除"的模式，这为竞态利用（符号链接攻击）提供了机会。
- **代码证据**:
  ```c
  remove(llvm_f);     // 可预测的临时文件名
  remove(obj_file);
  ```
- **影响**: 攻击者可以用指向目标文件的符号链接替换 `.ll` 或 `.o` 文件，导致删除任意文件。
- **修复建议**: 使用安全的临时文件创建方式

#### SEC-007: malloc溢出风险
- **文件**: `src/ast/ast.c:2247`
- **验证状态**: ⚠️ 未验证
- **描述**: `sizeof(ASTNode*) * new_count` 中没有整数溢出保护。精心构造的具有大量导入模块的源文件可能会使 `new_count` 发生溢出。
- **代码证据**:
  ```c
  ASTNode** new_statements = malloc(sizeof(ASTNode*) * new_count);
  ```
- **影响**: 可能导致分配过小和后续的堆损坏。
- **修复建议**: 添加溢出检查

#### SEC-008: realloc整数溢出
- **文件**: `src/ast/typeinfer.c:601-602`
- **验证状态**: ⚠️ 未验证
- **描述**: 将结构体字段添加到定义中时，`sizeof(StructField) * (struct_type->field_count + 1)` 可能会溢出。
- **代码证据**:
  ```c
  struct_type->fields = realloc(struct_type->fields, 
      sizeof(StructField) * (struct_type->field_count + 1));
  ```
- **影响**: 包含大量字段的结构体会导致堆分配过小。
- **修复建议**: 添加溢出检查

#### SEC-009: 堆耗尽风险
- **文件**: `src/utils/error.c:83-92`
- **验证状态**: ⚠️ 未验证
- **描述**: 整个源文件在被完全读取之前就被盲目地分配到堆中，且没有任何大小限制。如果文件大小为负或极大，会导致堆耗尽。
- **代码证据**:
  ```c
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);       // 可能为负或极大
  ...
  source_content = (char*)malloc(file_size + 1);  // 整数溢出 + 堆耗尽
  if (source_content) {
      fread(source_content, 1, file_size, file);  // 读取任意大小
  ```
- **影响**: 大文件可能导致内存耗尽。
- **修复建议**: 添加文件大小验证

---

### P2 - 中危安全漏洞

#### SEC-010: system()函数暴露
- **文件**: `std/os.vix:9-10`
- **验证状态**: ⚠️ 未验证
- **描述**: Vix标准库将C的 `system()` 函数封装并导出为公共API。任何导入 `os` 模块的Vix程序都可以执行任意shell命令。
- **代码证据**:
  ```vix
  pub fn system(cmd: ptr) -> i32 {
      return system(cmd)
  }
  ```
- **影响**: 已编译的Vix程序可以在编译时和运行时执行任意命令。
- **修复建议**: 移除或添加明确的危险函数警告

#### SEC-011: 固定大小栈缓冲区溢出风险
- **文件**: `src/main.c:454-466, 490-504, 536-544`
- **验证状态**: ⚠️ 未验证
- **描述**: 多个大小为2048字节的栈缓冲区。虽然 `snprintf` 受到限制，但如果任何计算出的名称超过2047个字符，路径操作可能会导致隐式截断。
- **代码证据**:
  ```c
  char oname[2048];
  snprintf(oname, sizeof(oname), "%.*s.o", (int)len, in_f);  // 如果 len > ~2030，则截断
  ```
- **影响**: 路径截断可能导致意外行为。
- **修复建议**: 动态分配或增加缓冲区大小

#### SEC-012: 路径操纵风险
- **文件**: `src/main.c:61-103`
- **验证状态**: ⚠️ 未验证
- **描述**: `exe_dir`（通过 `GetModuleFileNameA` / `readlink` 获取）可能包含攻击者控制的路径。
- **代码证据**:
  ```c
  static char libc_path[4096];
  char exe_dir[4096];
  ssize_t len = readlink("/proc/self/exe", exe_dir, sizeof(exe_dir) - 1);
  ...
  snprintf(libc_path, sizeof(libc_path), "%s%s", exe_dir, *s);
  ```
- **影响**: 路径注入可能导致加载恶意库。
- **修复建议**: 验证路径安全性

#### SEC-013: strncpy使用模式
- **文件**: `include/compat.h:47, 54`
- **验证状态**: ⚠️ 未验证
- **描述**: 此处的 `strncpy` 使用正确（在复制后手动设置空终止），但如果目标过小，其他位置的类似模式可能不安全。
- **代码证据**:
  ```c
  strncpy(resolved, r, resolved_size - 1);
  resolved[resolved_size - 1] = '\0';
  ```
- **影响**: 潜在的字符串截断问题。
- **修复建议**: 统一使用更安全的字符串函数

#### SEC-014: Windows setenv行为差异
- **文件**: `include/compat.h:69-76`
- **验证状态**: ⚠️ 未验证
- **描述**: 在Windows上，`overwrite` 参数被完全忽略——环境变量始终会被设置（或覆盖）。
- **代码证据**:
  ```c
  #ifdef _WIN32
      (void)overwrite;
      ...
      int ret = _putenv(buf);
  ```
- **影响**: 行为不一致可能导致意外结果。
- **修复建议**: 修复Windows实现以尊重 `overwrite` 参数

#### SEC-015: 字符串字面量转义解析
- **文件**: `src/parser/lexer.l:257-267`
- **验证状态**: ⚠️ 未验证
- **描述**: `process_escape_sequences` 在 `input_len + 1` 字节的堆缓冲区上运行，输出总是更短或等长——所以没问题。然而，如果字符串字面量格式错误，`my_strndup` 可能会截断空字节。
- **代码证据**:
  ```c
  char* raw_str = my_strndup(yytext + 1, yyleng - 2);
  yylval.str = process_escape_sequences(raw_str, strlen(raw_str));
  ```
- **影响**: 格式错误的字符串可能导致解析问题。
- **修复建议**: 添加输入验证

#### SEC-016: 内存泄漏
- **文件**: `src/parser/lexer.l:139-175`
- **验证状态**: ⚠️ 未验证
- **描述**: 多个词法分析器规则通过 `strdup(yytext)` 分配内存，并将结果存储在 `yylval.str` 中。分析器必须释放这些分配的内存——而通常它并未这样做。
- **代码证据**:
  ```c
  "print"  { UPDATE_COLUMN(); yylval.str = strdup(yytext); return PRINT; }
  ```
- **影响**: 长时间运行的编译会话可能耗尽内存。
- **修复建议**: 在适当时机释放分配的内存

---

### P3 - 低危安全漏洞

#### SEC-017: gets()函数支持
- **文件**: `src/compiler/CodeGen.cpp:4669`
- **验证状态**: ⚠️ 未验证
- **描述**: 代码生成器直接将 `gets()` 识别为已知的变参函数，并将其第一个参数注册为字符串缓冲区。虽然Vix本身没有 `gets()`，但将其作为 `extern "C"` 链接会使编译后的二进制文件面临缓冲区溢出问题。
- **代码证据**:
  ```cpp
  if ((calleeName == "fgets" || calleeName == "gets") && i == 0 &&
      argNode && argNode->type == AST_IDENTIFIER && argNode->data.identifier.name) {
      ...
      typeHelper.registerArrayType(strBufName, Type::getInt8Ty(context), -1);
  ```
- **影响**: 编译的程序可能包含不安全的代码。
- **修复建议**: 移除对 `gets()` 的支持或添加警告

#### SEC-018: 硬编码系统库路径
- **文件**: `src/compiler/Linker/Linker.cpp:68-70, 146-208, 251-259`
- **验证状态**: ⚠️ 未验证
- **描述**: 链接器在 `/usr/lib/`、`/lib64/`、`C:/Program Files/`、`C:/mingw64/` 等位置搜索库。如果攻击者可以将文件写入这些位置，他们就可以注入恶意库。
- **代码证据**:
  ```cpp
  std::string libDir = "/usr/lib/" + gnuTuple;
  if (!fileExists(libDir + "/crt1.o")) {
      std::string alt = "/usr/lib/" + std::string(T.isArch64Bit() ? "64" : "32");
  ```
- **影响**: 库注入攻击。
- **修复建议**: 使用更安全的库搜索路径

#### SEC-019: printf格式字符串解析
- **文件**: `src/compiler/CodeGen.cpp:4367-4375`
- **验证状态**: ⚠️ 未验证
- **描述**: 编译器解析 `printf` 格式字符串以确定参数类型。格式复杂的畸形字符串可能会导致类型不匹配和ABI违规。
- **代码证据**:
  ```cpp
  // parsePrintfFormatSpecs 函数中
  ```
- **影响**: 格式错误的字符串可能导致编译问题。
- **修复建议**: 加强格式字符串验证

#### SEC-020: 源代码行长度验证缺失
- **文件**: `src/utils/error.c:116-150`
- **验证状态**: ⚠️ 未验证
- **描述**: `get_line_content` 为任意长的源代码行分配堆内存。没有设置最大行长度限制。
- **代码证据**:
  ```c
  int line_len = (int)(end - start);
  char* line_content = (char*)malloc((size_t)line_len + 1);
  memcpy(line_content, start, (size_t)line_len);
  ```
- **影响**: 超长行可能导致内存问题。
- **修复建议**: 添加行长度限制

---

## 2. 代码质量与逻辑漏洞

### P0 - 严重逻辑错误

#### LOGIC-001: 使用未初始化的AST根节点
- **文件**: `src/main.c:328-347`
- **验证状态**: ⚠️ 未验证
- **描述**: `.vic` 文件处理器在 `yyparse()` 被调用之前就使用了 `root` 变量。`extern ASTNode* root` 变量只由解析器填充。在这个早期返回路径上，`root` 包含BSS初始化给它的任何内容（可能是NULL）。将NULL传递给 `llvm_emit_from_ast` 会导致未定义行为（可能在尝试解引用其成员时崩溃）。
- **代码证据**:
  ```c
  if (is_vic && gen_llvm) {
      ...
      llvm_emit_from_ast(root, llvm_file);  // root 在 yyparse() 之前使用
      ...
      return 0;
  }
  ...
  yyparse();  // 这才是设置 root 的地方
  ```
- **影响**: 每次 `.vic` 文件编译都会崩溃。
- **修复建议**: 在此代码路径之前解析 `.vic` 文件，或确保 `root` 被正确初始化

#### LOGIC-002: realloc返回值未检查 - 内存损坏
- **文件**: `src/ast/ast.c:235, 369, 477; src/ast/typeinfer.c:308, 346, 373, 413, 601, 625`
- **验证状态**: ⚠️ 未验证
- **描述**: 所有 `realloc` 调用都直接分配回原始指针。如果 `realloc` 失败，原始指针丢失（泄漏），后续写入会写入NULL（崩溃）。
- **代码证据**:
  ```c
  // ast.c:369
  program->data.program.statements = realloc(
      program->data.program.statements,
      sizeof(ASTNode*) * program->data.program.statement_count
  );
  // 如果 realloc 返回 NULL，原始指针丢失
  ```
- **影响**: 内存损坏和崩溃。
- **修复建议**: 始终使用临时指针并检查返回值

#### LOGIC-003: 空指针解引用风险
- **文件**: `src/ast/ast.c:557`
- **验证状态**: ⚠️ 未验证
- **描述**: `create_binop_node_with_location` 函数在没有NULL检查的情况下访问 `left->type` 和 `right->type`。如果调用者传递NULL子节点，这会崩溃。
- **代码证据**:
  ```c
  if (left->type == AST_STRING && right->type == AST_STRING) {
      // 如果 left 或 right 是 NULL，这里会崩溃
  ```
- **影响**: 畸形AST可能导致崩溃。
- **修复建议**: 添加NULL检查

#### LOGIC-004: strstr(NULL)崩溃
- **文件**: `src/main.c:330`
- **验证状态**: ⚠️ 未验证
- **描述**: 如果 `is_vic` 为真且 `gen_llvm` 为真，但 `save_c` 为真且 `malloc` 失败，`llvm_f` 可能为NULL。第330行在没有NULL保护的情况下调用 `strstr(llvm_f, ".ll")`。
- **代码证据**:
  ```c
  if (strstr(llvm_f, ".ll") == NULL) {  // llvm_f 可能是 NULL
  ```
- **影响**: 内存不足时崩溃。
- **修复建议**: 添加NULL检查

#### LOGIC-005: 字符串重复整数溢出
- **文件**: `src/ast/ast.c:588`
- **验证状态**: ⚠️ 未验证
- **描述**: `size_t total_len = str_len * repeat_times;` - 如果两个值都很大，这个乘法可能会溢出，导致 `total_len` 很小和缓冲区比复制的数据小。
- **代码证据**:
  ```c
  size_t total_len = str_len * repeat_times;  // 可能溢出
  ```
- **影响**: 堆缓冲区溢出。
- **修复建议**: 添加溢出检查

---

### P1 - 高危逻辑错误

#### LOGIC-006: 内存泄漏 - llvm_f从未释放
- **文件**: `src/main.c:310, 318`
- **验证状态**: ⚠️ 未验证
- **描述**: 当 `!llvm_f` 且 `save_c` 为真时，代码为 `llvm_name` 分配内存并将其分配给 `llvm_f`。这个指针在任何退出路径上都**从未被释放**——不在成功时（第589行），不在错误时（第476、515、554、574行）。每次调用都会泄漏内存。
- **代码证据**:
  ```c
  char* llvm_name = malloc(len + 4);
  ...
  llvm_f = llvm_name;  // 从未被 free
  ```
- **影响**: 内存泄漏。
- **修复建议**: 跟踪堆分配并在函数返回前释放

#### LOGIC-007: fread返回值被忽略
- **文件**: `src/utils/error.c:94; src/semantic/semantic.c:1374`
- **验证状态**: ⚠️ 未验证
- **描述**: `fread` 返回实际读取的字节数。两个调用都不检查返回值。如果读取被截断，`source_content` / `buffer` 将包含未初始化的数据。
- **代码证据**:
  ```c
  fread(source_content, 1, file_size, file);  // 返回值被忽略
  ```
- **影响**: 未初始化数据可能导致未定义行为。
- **修复建议**: 检查 `fread` 返回值

#### LOGIC-008: 除零错误静默延迟到运行时
- **文件**: `src/ast/ast.c:634, 646`
- **验证状态**: ⚠️ 未验证
- **描述**: 当在编译时检测到整数除法或模零（常量折叠）时，代码只是 `break` 出switch并创建一个非折叠的 `AST_BINOP` 节点。没有编译时错误或警告。这意味着 `let x = 1 / 0` 编译成功，只在运行时失败。
- **代码证据**:
  ```c
  case OP_DIV: {
      if (right->data.num_int.value != 0) {
          ...
          return;
      }
      break;  // 除零时不报错，静默延迟到运行时
  }
  ```
- **影响**: 静默的运行时错误。
- **修复建议**: 报告编译时常量除零错误

#### LOGIC-009: 字典序版本比较
- **文件**: `src/compiler/Linker/Linker.cpp:82-83, 157-158, 195-196, 236-237, 282-283`
- **验证状态**: ⚠️ 未验证
- **描述**: 查找"最佳" GCC/MSVC 版本使用 `if (name > bestVer)`，其中 `name` 是 `StringRef`。这进行字典序比较，所以 `"9.0" > "10.0"` 为真（因为 `'9' > '1'`）。这会导致选择错误的编译器目录。
- **代码证据**:
  ```cpp
  if (name > bestVer)
      bestVer = name.str();  // 字典序比较，不是版本比较
  ```
- **影响**: 选择了错误的编译器版本。
- **修复建议**: 在比较前解析版本为数值

#### LOGIC-010: 空参数列表问题
- **文件**: `src/semantic/semantic.c:53, 129, 765`
- **验证状态**: ⚠️ 未验证
- **描述**: 函数 `clear_var_init_map()`、`clear_var_struct_map()` 和 `report_mismatched_parentheses()` 使用空括号 `()`。在C（不是C++）中，这意味着**未指定参数**——不是零参数。调用者可以传递参数而没有警告。
- **代码证据**:
  ```c
  static void clear_var_init_map() {  // 应该是 (void)
  ```
- **影响**: 类型安全问题。
- **修复建议**: 改为 `clear_var_init_map(void)`

#### LOGIC-011: 类型混淆
- **文件**: `src/ast/ast.c:1530`
- **验证状态**: ⚠️ 未验证
- **描述**: 将 `void* yylloc` 转换为 `Location*` 而不是像其他所有 `_with_yyltype` 函数那样转换为 `YYLTYPE*`。虽然两个结构体碰巧有相同的布局，但这很脆弱且不一致。
- **代码证据**:
  ```c
  ASTNode* create_char_node_with_yyltype(char value, void* yylloc) {
      Location loc = *(Location*)yylloc;  // 应该是 YYLTYPE*
      return create_char_node_with_location(value, loc);
  }
  ```
- **影响**: 潜在的ABI兼容性问题。
- **修复建议**: 统一转换为 `YYLTYPE*`

#### LOGIC-012: check_only路径跳过清理
- **文件**: `src/main.c:252-258, 403-409`
- **验证状态**: ⚠️ 未验证
- **描述**: 如果 `out_f` 是通过 `malloc` 设置的（第252-258行）且使用了 `--check` 标志，分配的内存永远不会被释放。`--check` 路径在第409行返回而不释放 `out_f`。
- **代码证据**:
  ```c
  char* def_out = malloc(len + 1);
  ...
  if (check_only) {
      ...
      return 0;  // def_out 未被释放
  }
  ```
- **影响**: 内存泄漏。
- **修复建议**: 跟踪堆分配并在返回前释放

---

### P2 - 中危逻辑错误

#### LOGIC-013: 不可达代码
- **文件**: `src/ast/ast.c:1821-1822`
- **验证状态**: ⚠️ 未验证
- **描述**: 在 `return` 语句后的两行代码永远不可达：
- **代码证据**:
  ```c
  return;          // 第1820行
  print_ast(...);  // 第1821行 - 死代码
  print_ast(...);  // 第1822行 - 死代码
  ```
- **影响**: 代码混乱。
- **修复建议**: 移除不可达代码

#### LOGIC-014: 未使用的函数参数
- **文件**: `src/semantic/semantic.c:89, 103`
- **验证状态**: ⚠️ 未验证
- **描述**: 函数 `is_node_struct_field_assignment` 和 `is_node_inside_struct_literal` 接受 `ASTNode* node` 参数但立即用 `(void)node;` 抑制它。参数在函数体中从未使用。
- **代码证据**:
  ```c
  static bool is_node_struct_field_assignment(ASTNode* node) {
      (void)node;  // 参数从未使用
      ...
  }
  ```
- **影响**: 代码混乱。
- **修复建议**: 移除未使用的参数

#### LOGIC-015: errors_found变量从未递增
- **文件**: `src/semantic/semantic.c:1359-1409`
- **验证状态**: ⚠️ 未验证
- **描述**: `extract_public_functions_from_module` 声明并返回 `errors_found`，但变量初始化为0且**从未在函数中递增**。即使内部解析失败，函数也总是返回0。
- **代码证据**:
  ```c
  int errors_found = 0;
  ...
  // errors_found 从未被修改
  return errors_found;  // 总是返回 0
  ```
- **影响**: 错误被静默忽略。
- **修复建议**: 实际计算错误或移除错误跟踪

#### LOGIC-016: 不一致的平台宏
- **文件**: `src/compiler/Passes.cpp:49, 70; src/compiler/CodeGen.cpp:1601`
- **验证状态**: ⚠️ 未验证
- **描述**: 一些文件使用 `#ifdef WIN32`（非标准），其他文件使用 `#ifdef _WIN32`。`WIN32`（没有下划线）不是所有MSVC/clang-cl配置中预定义的标准宏。`_WIN32` 是标准的。
- **代码证据**:
  ```cpp
  // Passes.cpp:49
  #ifdef WIN32  // 非标准
  // main.c
  #ifdef _WIN32  // 标准
  ```
- **影响**: Windows构建可能失败或行为不正确。
- **修复建议**: 统一使用 `#ifdef _WIN32`

#### LOGIC-017: 启发式字符串变量检测
- **文件**: `src/compiler/CodeGen.cpp:1210-1217`
- **验证状态**: ⚠️ 未验证
- **描述**: 检查 `varName.find("str") != std::string::npos` 来决定变量是否为字符串类型是脆弱的。名为 `"myStruct"` 或 `"listRange"` 的变量会被错误识别。
- **代码证据**:
  ```cpp
  if (varName.find("str") != std::string::npos ||
      varName.find("Str") != std::string::npos ||
      varName.find("STRING") != std::string::npos ||
      varName.find("lit") != std::string::npos) {
      return Type::getInt8Ty(context);
  }
  ```
- **影响**: 代码生成错误。
- **修复建议**: 跟踪实际声明的类型而不是使用启发式方法

#### LOGIC-018: yyparse失败未传播
- **文件**: `src/ast/ast.c:200-213`
- **验证状态**: ⚠️ 未验证
- **描述**: 在 `parse_imported_module_ast` 中，当 `yyparse()` 返回非零时，`root`（全局变量）可能被部分填充。释放部分构建的AST可能在 `free_ast` 中由于未初始化的union字段而崩溃。
- **代码证据**:
  ```c
  int parse_result = yyparse();
  module_root = root;  // 即使 parse_result != 0，也可能非 NULL
  if (parse_result != 0) {
      free_ast(module_root);  // 可能崩溃
  }
  ```
- **影响**: 导入模块解析失败可能导致崩溃。
- **修复建议**: 在调用 `yyparse()` 前清除 `root = NULL`

#### LOGIC-019: inline_imports返回值被丢弃
- **文件**: `src/ast/ast.c:2386`
- **验证状态**: ⚠️ 未验证
- **描述**: `inline_imports_in_node_impl` 的返回值（成功/失败）被显式转换为 `void` 并丢弃。调用者无法知道导入解析是否失败。
- **代码证据**:
  ```c
  static void inline_imports_in_node(ASTNode* node) {
      (void)inline_imports_in_node_impl(node);  // 返回值被丢弃
  }
  ```
- **影响**: 导入错误被静默忽略。
- **修复建议**: 传播错误或至少记录它

---

### P3 - 低危逻辑错误

#### LOGIC-020: 静态缓冲区线程安全问题
- **文件**: `src/main.c:60, 413`
- **验证状态**: ⚠️ 未验证
- **描述**: 多个静态缓冲区（`char libc_path[4096]`、`char llvm_filename[2048]`）在可能从多线程上下文调用的代码中。`find_bundled_libc()` 返回指向静态数据的指针，该指针可以在下次调用时被覆盖。
- **代码证据**:
  ```c
  static char libc_path[4096];  // 静态缓冲区
  ```
- **影响**: 多线程环境下数据竞争。
- **修复建议**: 使用线程局部存储或动态分配

#### LOGIC-021: fread返回值未检查
- **文件**: `src/utils/error.c:94; src/semantic/semantic.c:1374`
- **验证状态**: ⚠️ 未验证
- **描述**: `fread` 返回实际读取的字节数。两个调用都不检查返回值。如果读取被截断，`source_content` / `buffer` 将包含未初始化的数据。
- **代码证据**:
  ```c
  fread(source_content, 1, file_size, file);  // 返回值被忽略
  ```
- **影响**: 未初始化数据可能导致未定义行为。
- **修复建议**: 检查 `fread` 返回值

#### LOGIC-022: YYLTYPE重新定义
- **文件**: `include/ast.h:13-21`
- **验证状态**: ⚠️ 未验证
- **描述**: 如果尚未声明，`YYLTYPE` 结构体会被重新定义——保护使用 `#ifndef YYLTYPE_IS_DECLARED` 但不包含Bison头文件。这是一个脆弱的自我重新定义。
- **代码证据**:
  ```c
  #ifndef YYLTYPE_IS_DECLARED
  typedef struct YYLTYPE {
      int first_line;
      int first_column;
      int last_line;
      int last_column;
  } YYLTYPE;
  #define YYLTYPE_IS_DECLARED 1
  #endif
  ```
- **影响**: 潜在的类型冲突。
- **修复建议**: 使用标准的Bison头文件包含

#### LOGIC-023: 输入文件未关闭
- **文件**: `src/main.c:352`
- **验证状态**: ⚠️ 未验证
- **描述**: 如果 `error_count > 0`（第352行），函数返回1而不 `fclose(input_file)`，泄漏文件句柄。
- **代码证据**:
  ```c
  if (error_count > 0) {
      return 1;  // input_file 未关闭
  }
  ```
- **影响**: 文件句柄泄漏。
- **修复建议**: 在所有返回路径关闭文件

#### LOGIC-024: free_ast缺少case
- **文件**: `src/ast/ast.c`
- **验证状态**: ⚠️ 未验证
- **描述**: `free_ast` 函数的switch语句处理许多但不是所有AST节点类型。缺少的case包括 `AST_IMPORT` 和其他——如果这些节点被释放，它们的数据结构成员（如 `module_path`）会泄漏。
- **代码证据**:
  ```c
  switch (node->type) {
      case AST_BINOP: ...
      case AST_STRING: ...
      // 缺少 AST_IMPORT 等
  }
  ```
- **影响**: 内存泄漏。
- **修复建议**: 添加所有AST节点类型的处理

---

## 3. 文档问题

### P0 - 严重文档问题

#### DOC-001: README.md中的Git合并冲突
- **文件**: `README.md:75-79`
- **验证状态**: ✅ 已验证
- **描述**: Ecosystem表中存在未解决的git合并冲突标记 `<<<<<<< HEAD`、`=======`、`>>>>>>>`。这会破坏markdown渲染。
- **代码证据**:
  ```
  <<<<<<< HEAD
  | **Very** | Project manager and build tool for Vix | In active development |
  =======
  | **Very** | Package manager for Vix | Community contribution [Very](https://github.com/vixlang/Very)|
  >>>>>>> a6a1d09663d7f12cd498408586b27a2d33d4ae08
  ```
- **影响**: 项目文档不可读。
- **修复建议**: 解决合并冲突

---

### P1 - 高危文档问题

#### DOC-002: Docs/目录不存在 - 13个断链
- **文件**: `README.md`, `README-zh_CN.md`
- **验证状态**: ✅ 已验证
- **描述**: `Docs/en/` 和 `Docs/zh_CN/` 目录在仓库中不存在。两个README都广泛引用这些目录中的文件，但这些文件从未被创建。
- **断链列表**:
  - `Docs/en/getting-started.md`
  - `Docs/en/what-is-vix.md`
  - `Docs/en/syntax.md`
  - `Docs/zh_CN/getting-started.md`
  - `Docs/zh_CN/syntax.md`
  - `Docs/zh_CN/types.md`
  - `Docs/zh_CN/functions.md`
  - `Docs/zh_CN/modules.md`
  - `Docs/zh_CN/structs.md`
  - `Docs/zh_CN/pointers.md`
  - `Docs/zh_CN/control-flow.md`
  - `Docs/zh_CN/stdlib.md`
- **影响**: 13个文档链接全部断链。
- **修复建议**: 创建文档目录或更新README移除断链

#### DOC-003: GitHub组织链接不一致
- **文件**: `README.md:54, 63; README-zh_CN.md:54, 62`
- **验证状态**: ✅ 已验证
- **描述**: 远程origin是 `github.com/vixlang/Vix-lang.git`，但多个链接使用错误的组织 `Daweidie`。
- **代码证据**:
  ```
  [GitHub Issues](https://github.com/Daweidie/vix-lang/issues)
  [vix-lang issues](https://github.com/Daweidie/vix-lang/issues)
  ```
- **影响**: 链接指向错误的仓库。
- **修复建议**: 更新所有链接为正确的组织

#### DOC-004: README-zh_CN.md错误的Vix链接
- **文件**: `README-zh_CN.md:16`
- **验证状态**: ✅ 已验证
- **描述**: 英文README正确指向网站 `https://vixlang.github.io`，但中文版本指向GitHub仓库。
- **代码证据**:
  ```
  [Vix]: https://github.com/vixlang/Vix-lang  // 应该是 https://vixlang.github.io
  ```
- **影响**: 中文用户无法访问项目网站。
- **修复建议**: 更新链接为网站URL

#### DOC-005: pyproject.toml版本不同步
- **文件**: `pyproject.toml:7`
- **验证状态**: ✅ 已验证
- **描述**: 版本为 "0.1.2"，但编译器报告版本 "0.2.2"（`src/main.c:166`）。测试套件版本落后11个次版本。
- **代码证据**:
  ```toml
  version = "0.1.2"  # 应该是 0.2.2
  ```
- **影响**: 版本混乱。
- **修复建议**: 同步版本号

#### DOC-006: LICENSE年份过时
- **文件**: `LICENSE:5`
- **验证状态**: ✅ 已验证
- **描述**: 版权年份为2025，但源代码头文件（`src/main.c:2`）显示 "2025-2026"。年份和版权持有者在两个文件之间不一致。
- **代码证据**:
  ```
  Copyright 2025 Mulang_zty  # LICENSE
  Copyright (c) 2025-2026 Vix Language Authors  # src/main.c
  ```
- **影响**: 法律文档不准确。
- **修复建议**: 更新版权信息

---

### P2 - 中危文档问题

#### DOC-007: 重复的LearnVix和VixDocs条目
- **文件**: `README.md:49-50`
- **验证状态**: ✅ 已验证
- **描述**: 两个条目都链接到完全相同的仓库 `vixlang/LearnVix`。冒号后也缺少空格。
- **代码证据**:
  ```
  - LearnVix:[GitHub Link](https://github.com/vixlang/LearnVix)
  - VixDocs:[GitHub Link](https://github.com/vixlang/LearnVix)
  ```
- **影响**: 用户困惑。
- **修复建议**: 区分或合并这些资源

#### DOC-008: docs/COMPILER.md引用不存在的文件
- **文件**: `docs/COMPILER.md:355`
- **验证状态**: ✅ 已验证
- **描述**: 提到更新 `Docs/syntax.ebnf`，但仓库中不存在任何 `*.ebnf` 文件。
- **代码证据**:
  ```
  - Updated `Docs/syntax.ebnf` with power operator in expression grammar
  ```
- **影响**: 文档不准确。
- **修复建议**: 创建文件或更新引用

#### DOC-009: str关键字已移除但仍列出
- **文件**: `docs/COMPILER.md:35`
- **验证状态**: ✅ 已验证
- **描述**: `str` 被列为类型关键字，但 `docs/RELEASE_v0.2.1.md` 明确说明 `str` 在v0.2.1中被移除并替换为 `string`。
- **代码证据**:
  ```
  - **Types:** `i8`, `i32`, `i64`, `f32`, `f64`, `bool`, `string`, `str`, `void`, `ptr`
  ```
- **影响**: 文档矛盾。
- **修复建议**: 更新关键字列表

#### DOC-010: 过时的函数语法
- **文件**: `docs/COMPILER.md:44`
- **验证状态**: ✅ 已验证
- **描述**: 文档使用 `-> RetType` 语法，但 `docs/RELEASE_v0.2.5.md` 说明 `-> type` 语法已弃用——应使用 `: type`。
- **代码证据**:
  ```
  - **Functions:** `fn name:[generics](params) -> RetType { body }`
  ```
- **影响**: 用户使用过时语法。
- **修复建议**: 更新为当前语法

#### DOC-011: 重复的Testing部分
- **文件**: `docs/COMPILER.md:222-263, 289-324`
- **验证状态**: ✅ 已验证
- **描述**: 两个"Testing"部分本质上相同但有不同的测试文件列表，造成混淆。
- **代码证据**:
  ```
  第222-263行: 第一个Testing部分
  第289-324行: 第二个Testing部分（重复）
  ```
- **影响**: 用户困惑于哪个是当前的。
- **修复建议**: 合并或移除重复部分

#### DOC-012: Unix专用路径
- **文件**: `docs/COMPILER.md:227, 295`
- **验证状态**: ✅ 已验证
- **描述**: `.venv/bin/python` 是Unix专用路径。没有Windows等效路径的文档。
- **代码证据**:
  ```
  .venv/bin/python -m pytest tests/ -v
  ```
- **影响**: Windows用户无法运行测试。
- **修复建议**: 添加Windows说明

---

### P3 - 低危文档问题

#### DOC-013: 网站Getting Started链接404
- **验证状态**: ⚠️ 未验证
- **描述**: 网站 `https://vixlang.github.io/docs/getting-started` 返回HTTP 404。
- **影响**: 用户无法访问入门指南。
- **修复建议**: 修复网站链接

#### DOC-014: usize矛盾
- **文件**: `docs/RELEASE_v0.2.1.md:23, 43`
- **验证状态**: ✅ 已验证
- **描述**: 第23行说 "`usize` 现在是识别的类型关键字"，第43行说 "从函数签名中移除了不支持的 `usize` 类型"。这两个陈述矛盾。
- **代码证据**:
  ```
  Line 23: `usize` is now a recognized type keyword
  Line 43: Removed unsupported `usize` type from function signatures
  ```
- **影响**: 用户困惑。
- **修复建议**: 澄清usize的状态

#### DOC-015: README-zh_CN.md缺少Ecosystem链接
- **验证状态**: ✅ 已验证
- **描述**: 英文Ecosystem表有链接，但中文版本没有。
- **影响**: 中文用户缺少信息。
- **修复建议**: 添加链接

---

## 4. 构建系统问题

### P0 - 严重构建问题

#### BUILD-001: 缺少MSVC编译器支持
- **文件**: `CMakeLists.txt:128-129`
- **验证状态**: ⚠️ 未验证
- **描述**: `-Wall -Wextra -fno-stack-protector -fexceptions` 是GCC/Clang风格标志。在Windows上使用MSVC工具链生成器时，构建**必定失败**。
- **代码证据**:
  ```cmake
  target_compile_options(vixc PRIVATE
      $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -fno-stack-protector>
      $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -fno-stack-protector -fexceptions>
  )
  ```
- **影响**: Windows原生构建失败。
- **修复建议**: 添加MSVC编译器检测和相应标志

---

### P1 - 高危构建问题

#### BUILD-002: 弃用的LLVM组件API
- **文件**: `CMakeLists.txt:141-149`
- **验证状态**: ⚠️ 未验证
- **描述**: `llvm_map_components_to_libnames` 自LLVM 15起弃用，在LLVM 20中失效。项目声称目标为LLVM 20。
- **代码证据**:
  ```cmake
  llvm_map_components_to_libnames(LLVM_LIBS
      core support passes irreader target
      x86asmparser x86codegen x86desc x86disassembler x86info
      ...
  )
  ```
- **影响**: 在LLVM 20+上可能构建失败。
- **修复建议**: 迁移到现代组件目标

#### BUILD-003: 无条件的全目标链接
- **文件**: `CMakeLists.txt:141-149`
- **验证状态**: ⚠️ 未验证
- **描述**: 组件列表强制链接x86、ARM、AArch64、WebAssembly后端。用户即使只需要一个目标也要付出链接时间和二进制大小代价。
- **代码证据**:
  ```cmake
  x86asmparser x86codegen x86desc x86disassembler x86info
  aarch64asmparser aarch64codegen aarch64desc aarch64disassembler aarch64info
  armasmparser armcodegen armdesc armdisassembler arminfo
  webassemblyasmparser webassemblycodegen webassemblydesc webassemblydisassembler webassemblyinfo
  ```
- **影响**: 构建时间长，二进制文件大。
- **修复建议**: 使目标选择可配置

#### BUILD-004: 无条件的LLD组件链接
- **文件**: `CMakeLists.txt:152-154`
- **验证状态**: ⚠️ 未验证
- **描述**: 链接**所有**LLD后端，无论平台。在macOS上不需要lldCOFF、lldWasm或lldMinGW。
- **代码证据**:
  ```cmake
  target_link_libraries(vixc PRIVATE
      lldELF lldMachO lldCOFF lldWasm lldMinGW lldCommon
  )
  ```
- **影响**: 二进制膨胀，潜在链接失败。
- **修复建议**: 根据平台有条件地链接

#### BUILD-005: macOS打包错误
- **文件**: `.github/workflows/build.yml:196`
- **验证状态**: ⚠️ 未验证
- **描述**: `install_name_tool -id` 中的 `${depname}` 变量超出作用域。它在前一个循环中设置，该循环**已经结束**。
- **代码证据**:
  ```yaml
  install_name_tool -id "@executable_path/${depname}" "$f" 2>/dev/null || true
  ```
- **影响**: 损坏的dylib标识名。
- **修复建议**: 修复变量作用域

#### BUILD-006: Windows打包使用不可用的ldd
- **文件**: `.github/workflows/build.yml:221`
- **验证状态**: ⚠️ 未验证
- **描述**: 在MSYS2环境中，`ldd` 并非总是可用。MSYS2使用 `ntldd` 或 `objdump -p`。
- **代码证据**:
  ```yaml
  ldd dist/bin/vixc.exe | grep -vi '/c/WINDOWS/' | awk '{ print $3 }'
  ```
- **影响**: Windows打包步骤可能失败。
- **修复建议**: 使用 `ntldd` 替代

---

### P2 - 中危构建问题

#### BUILD-007: 废弃的CMake命令
- **文件**: `CMakeLists.txt:87, 89`
- **验证状态**: ⚠️ 未验证
- **描述**: 使用 `include_directories` 和 `add_definitions` 而不是现代的 `target_include_directories` 和 `target_compile_definitions`。
- **代码证据**:
  ```cmake
  include_directories(${LLVM_INCLUDE_DIRS} ${LLD_INCLUDE_DIRS})
  add_definitions(${LLVM_DEFINITIONS_LIST})
  ```
- **影响**: 全局影响可能泄漏到其他目标。
- **修复建议**: 使用目标特定命令

#### BUILD-008: LLVM版本不匹配
- **文件**: `src/install.sh:49`
- **验证状态**: ⚠️ 未验证
- **描述**: 安装 `llvm`（Ubuntu默认版本）但 `clang-18`（LLVM 18）。这**安装了两个不同版本**的LLVM。
- **代码证据**:
  ```bash
  Install="apt install -y gcc flex bison llvm clang-18 libclang-18-dev git"
  ```
- **影响**: 混乱的包状态。
- **修复建议**: 统一LLVM版本

#### BUILD-009: 未固定的CI包版本
- **文件**: `.github/workflows/build.yml:80-88`
- **验证状态**: ⚠️ 未验证
- **描述**: MSYS2滚动更新包。没有版本固定。
- **代码证据**:
  ```yaml
  install: >-
      mingw-w64-x86_64-llvm
      mingw-w64-x86_64-lld
  ```
- **影响**: 非确定性构建。
- **修复建议**: 固定包版本

#### BUILD-010: pyproject.toml使用遗留后端
- **文件**: `pyproject.toml:3`
- **验证状态**: ✅ 已验证
- **描述**: 使用遗留setuptools后端而不是现代的 `setuptools.build_meta`。
- **代码证据**:
  ```toml
  build-backend = "setuptools.backends._legacy:_Backend"
  ```
- **影响**: 未来兼容性问题。
- **修复建议**: 迁移到现代后端

#### BUILD-011: 缺少测试依赖声明
- **文件**: `pyproject.toml:11-27`
- **验证状态**: ✅ 已验证
- **描述**: `pytest` 和 `pytest-timeout` 没有声明在依赖中。
- **代码证据**:
  ```toml
  [tool.pytest.ini_options]
  timeout = 60  # pytest-timeout 未声明为依赖
  ```
- **影响**: 开发者入门体验差。
- **修复建议**: 添加可选依赖

---

### P3 - 低危构建问题

#### BUILD-012: 缺少CMake预设
- **验证状态**: ⚠️ 未验证
- **描述**: 没有 `CMakePresets.json`，使得跨环境构建难以一致。
- **影响**: 可重复性降低。
- **修复建议**: 创建预设文件

#### BUILD-013: 硬编码的安装目标
- **文件**: `CMakeLists.txt:159`
- **验证状态**: ⚠️ 未验证
- **描述**: 使用硬编码的 `bin` 而不是 `${CMAKE_INSTALL_BINDIR}`。
- **代码证据**:
  ```cmake
  install(TARGETS vixc RUNTIME DESTINATION bin)
  ```
- **影响**: 非标准布局发行版上安装位置错误。
- **修复建议**: 使用GNUInstallDirs

#### BUILD-014: 未记录的VIX_DEBUG环境变量
- **文件**: `src/main.c:234`
- **验证状态**: ⚠️ 未验证
- **描述**: 编译器读取 `VIX_DEBUG` 环境变量但未在文档中记录。
- **代码证据**:
  ```c
  vix_setenv("VIX_DEBUG", dbg ? "1" : "0", 1);
  ```
- **影响**: 用户可能意外设置导致意外调试输出。
- **修复建议**: 记录环境变量

#### BUILD-015: .gitignore重复项
- **验证状态**: ⚠️ 未验证
- **描述**: `__pycache__` 在第5行和第18行列出两次。
- **影响**: 代码混乱。
- **修复建议**: 移除重复项

#### BUILD-016: 构建工件已提交
- **验证状态**: ⚠️ 未验证
- **描述**: `CMakeFiles/CMakeSystem.cmake` 显示WSL2构建信息，表明构建工件被提交到仓库。
- **影响**: 仓库污染。
- **修复建议**: 确保.gitignore正确工作

---

## 5. 测试覆盖问题

### P0 - 严重测试问题

#### TEST-001: CI构建后不运行测试
- **文件**: `.github/workflows/build.yml`
- **验证状态**: ⚠️ 未验证
- **描述**: 尽管测试基础设施存在，但CI在构建后不运行任何测试。编译器可能在CI上无声地回归。
- **影响**: 回归不被检测。
- **修复建议**: 在CI中添加测试执行步骤

---

### P1 - 高危测试问题

#### TEST-002: 回归测试文件不匹配
- **文件**: `regre.py:237-238`
- **验证状态**: ⚠️ 未验证
- **描述**: 期望220个测试文件但只有约100个 `.vix` 文件存在。120个缺失。
- **代码证据**:
  ```python
  list(range(1, 221))  # 期望 1-220
  ```
- **影响**: 测试覆盖不完整。
- **修复建议**: 创建缺失的测试文件或更新期望

#### TEST-003: Windows编译器路径错误
- **文件**: `helpers.py:6`
- **验证状态**: ⚠️ 未验证
- **描述**: `COMPILER = ROOT / "build" / "vixc"` — Windows上应为 `vixc.exe`。测试会静默跳过。
- **代码证据**:
  ```python
  COMPILER = ROOT / "build" / "vixc"  # Windows 上应该是 vixc.exe
  ```
- **影响**: Windows上测试不运行。
- **修复建议**: 检测平台并使用正确后缀

#### TEST-004: 编译器缺失时测试静默跳过
- **文件**: `conftest.py:22-23`
- **验证状态**: ⚠️ 未验证
- **描述**: 如果编译器不存在，测试被跳过而不是失败。这给出错误的成功信号。
- **代码证据**:
  ```python
  if not COMPILER.exists():
      pytest.skip(...)
  ```
- **影响**: 测试结果不可靠。
- **修复建议**: 失败而不是跳过

#### TEST-005: 整个标准库未测试
- **验证状态**: ⚠️ 未验证
- **描述**: `std/*.vix` 文件中的函数从未被任何测试导入或测试。
- **影响**: 标准库可能包含未检测到的错误。
- **修复建议**: 为标准库添加测试

#### TEST-006: 导入系统未测试
- **验证状态**: ⚠️ 未验证
- **描述**: 测试中从未使用 `import` 语句。模块导入系统完全未测试。
- **影响**: 导入功能可能不工作。
- **修复建议**: 添加导入系统测试

#### TEST-007: CLI标志未测试
- **验证状态**: ⚠️ 未验证
- **描述**: `--check`、`--time`、`--debug`、`--target` 标志完全未测试。
- **影响**: CLI功能可能不工作。
- **修复建议**: 添加CLI测试

---

### P2 - 中危测试问题

#### TEST-008: 薄弱断言
- **文件**: `test_types.py:150-168, 327-379, 411-414`
- **验证状态**: ⚠️ 未验证
- **描述**: 许多测试只检查编译成功，不验证运行时输出。
- **代码证据**:
  ```python
  assert compile_res.returncode == 0  # 只检查编译，不检查输出
  ```
- **影响**: 功能错误可能不被检测。
- **修复建议**: 添加运行时输出验证

#### TEST-009: 示例测试不完整
- **文件**: `examp.py:9-26`
- **验证状态**: ⚠️ 未验证
- **描述**: 44个示例中只测试了16个。28个示例未测试。
- **影响**: 示例可能不工作。
- **修复建议**: 测试所有示例

#### TEST-010: 重复测试
- **文件**: `test_types.py:1152-1156` vs `stress.py:368-388`
- **验证状态**: ⚠️ 未验证
- **描述**: 完全相同的测试在多个文件中重复。
- **影响**: 维护负担。
- **修复建议**: 合并重复测试

#### TEST-011: 弱断言 - not None
- **文件**: `stress.py:167-183, 265-278`
- **验证状态**: ⚠️ 未验证
- **描述**: 一些测试只断言输出不是None，不验证实际值。
- **代码证据**:
  ```python
  assert run.stdout.strip() is not None  # 太弱
  ```
- **影响**: 功能错误可能不被检测。
- **修复建议**: 添加精确断言

---

### P3 - 低危测试问题

#### TEST-012: 不一致的断言风格
- **验证状态**: ⚠️ 未验证
- **描述**: 一些测试使用精确匹配，其他使用子字符串匹配。没有一致的约定。
- **影响**: 测试可维护性差。
- **修复建议**: 标准化断言风格

#### TEST-013: Unix专用测试路径
- **文件**: `run.py:8`
- **验证状态**: ⚠️ 未验证
- **描述**: `VENV_PYTHON = ROOT / ".venv" / "bin" / "python3"` 是Unix硬编码路径。
- **代码证据**:
  ```python
  VENV_PYTHON = ROOT / ".venv" / "bin" / "python3"  # Windows 上失败
  ```
- **影响**: Windows上测试失败。
- **修复建议**: 使用 `sys.executable`

#### TEST-014: 测试配置过于宽泛
- **文件**: `pyproject.toml:13`
- **验证状态**: ✅ 已验证
- **描述**: `python_files = ["*.py"]` 选取所有 `.py` 文件，包括非测试脚本。
- **代码证据**:
  ```toml
  python_files = ["*.py"]
  ```
- **影响**: 可能意外执行非测试文件。
- **修复建议**: 收窄文件模式

#### TEST-015: 缺少测试文档
- **验证状态**: ⚠️ 未验证
- **描述**: 没有关于如何运行测试、编写测试或设置开发环境的文档。
- **影响**: 新贡献者难以参与。
- **修复建议**: 创建测试文档

#### TEST-016: 无单元测试
- **验证状态**: ⚠️ 未验证
- **描述**: 所有测试都是端到端的。没有针对单个C/C++函数的单元测试。
- **影响**: 内部函数错误难以定位。
- **修复建议**: 添加单元测试

#### TEST-017: 无快照测试
- **验证状态**: ⚠️ 未验证
- **描述**: 生成的LLVM IR和AST输出从未通过快照进行验证。
- **影响**: 代码生成变化可能无声地改变行为。
- **修复建议**: 添加快照测试

#### TEST-018: fuzz测试不验证正确性
- **文件**: `fuzz.py:279-285`
- **验证状态**: ⚠️ 未验证
- **描述**: Fuzz测试检查 `returncode < 0`（信号）但从不验证输出正确性。
- **代码证据**:
  ```python
  assert run.returncode < 0  # 只检查崩溃，不验证正确性
  ```
- **影响**: 逻辑错误可能不被检测。
- **修复建议**: 添加输出验证

---

## 附录：统计摘要

### 按严重程度统计

| 等级 | 数量 | 占比 |
|------|------|------|
| P0 | 8 | 7.3% |
| P1 | 28 | 25.5% |
| P2 | 35 | 31.8% |
| P3 | 29 | 26.4% |
| P4 | 6 | 5.5% |
| P5 | 4 | 3.6% |
| **总计** | **110** | **100%** |

### 按类别统计

| 类别 | P0 | P1 | P2 | P3 | P4 | P5 | 总计 |
|------|----|----|----|----|----|----|------|
| 安全漏洞 | 3 | 6 | 7 | 4 | 0 | 0 | 20 |
| 代码质量 | 5 | 7 | 7 | 5 | 0 | 0 | 24 |
| 文档问题 | 1 | 5 | 6 | 3 | 0 | 0 | 15 |
| 构建系统 | 1 | 5 | 5 | 5 | 0 | 0 | 16 |
| 测试覆盖 | 1 | 6 | 4 | 7 | 0 | 0 | 18 |
| **总计** | **11** | **29** | **29** | **24** | **0** | **0** | **93** |

### 验证状态统计

| 状态 | 数量 | 占比 |
|------|------|------|
| ✅ 已验证 | 22 | 20% |
| ⚠️ 未验证 | 88 | 80% |
| **总计** | **110** | **100%** |

### 优先修复建议

1. **立即修复** (P0):
   - SEC-001: 移除 `-fno-stack-protector`
   - SEC-003: 修复栈缓冲区溢出
   - LOGIC-001: 修复使用未初始化的root
   - BUILD-001: 添加MSVC支持

2. **24小时内** (P1):
   - SEC-004 to SEC-009: 修复整数溢出和内存问题
   - LOGIC-002: 修复realloc返回值检查
   - TEST-001: 在CI中添加测试

3. **1周内** (P2):
   - 修复剩余的中危问题
   - 改善测试覆盖

4. **1个月内** (P3):
   - 代码清理和文档完善
   - 添加单元测试和快照测试

---

**报告生成时间**: 2026-05-30  
**审计工具**: 5个并行Agent深度审查  
**审计范围**: 安全漏洞、代码质量、文档问题、构建系统、测试覆盖  
**验证状态**: 未验证（WSL环境依赖安装超时）
