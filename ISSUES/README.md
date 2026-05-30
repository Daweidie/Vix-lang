# Vix-lang 审计问题快速索引

**生成时间**: 2026-05-30  
**完整报告**: [SECURITY_AUDIT_REPORT.md](./SECURITY_AUDIT_REPORT.md)

---

## P0 - 立即修复 (8个)

| ID | 类别 | 文件 | 问题 |
|----|------|------|------|
| SEC-001 | 安全 | `CMakeLists.txt:128-129` | 栈保护被禁用 `-fno-stack-protector` |
| SEC-002 | 安全 | `src/install.sh:86` | 命令注入 `eval "$cmd"` |
| SEC-003 | 安全 | `src/main.c:328-333` | 栈缓冲区溢出 `llvm_filename[256]` |
| LOGIC-001 | 逻辑 | `src/main.c:328-347` | 使用未初始化的AST root |
| LOGIC-002 | 逻辑 | 多个文件 | realloc返回值未检查 |
| LOGIC-003 | 逻辑 | `src/ast/ast.c:557` | 空指针解引用风险 |
| LOGIC-004 | 逻辑 | `src/main.c:330` | strstr(NULL)崩溃 |
| BUILD-001 | 构建 | `CMakeLists.txt:128-129` | 缺少MSVC编译器支持 |

---

## P1 - 24小时内修复 (28个)

### 安全漏洞 (6个)
- SEC-004: 字符串乘法整数溢出 → 堆溢出
- SEC-005: 容量倍增整数溢出
- SEC-006: TOCTOU竞态条件
- SEC-007: malloc溢出风险
- SEC-008: realloc整数溢出
- SEC-009: 堆耗尽风险

### 代码质量 (7个)
- LOGIC-005: 字符串重复整数溢出
- LOGIC-006: llvm_f内存泄漏
- LOGIC-007: fread返回值被忽略
- LOGIC-008: 除零错误静默延迟
- LOGIC-009: 字典序版本比较
- LOGIC-010: 空参数列表问题
- LOGIC-011: 类型混淆

### 文档问题 (5个)
- DOC-002: Docs/目录不存在 - 13个断链
- DOC-003: GitHub组织链接不一致
- DOC-004: README-zh_CN.md错误的Vix链接
- DOC-005: pyproject.toml版本不同步
- DOC-006: LICENSE年份过时

### 构建系统 (5个)
- BUILD-002: 弃用的LLVM组件API
- BUILD-003: 无条件的全目标链接
- BUILD-004: 无条件的LLD组件链接
- BUILD-005: macOS打包错误
- BUILD-006: Windows打包使用不可用的ldd

### 测试覆盖 (6个)
- TEST-001: CI构建后不运行测试
- TEST-002: 回归测试文件不匹配
- TEST-003: Windows编译器路径错误
- TEST-004: 编译器缺失时测试静默跳过
- TEST-005: 整个标准库未测试
- TEST-006: 导入系统未测试

---

## P2 - 1周内修复 (29个)

### 安全漏洞 (7个)
- SEC-010: system()函数暴露
- SEC-011: 固定大小栈缓冲区溢出风险
- SEC-012: 路径操纵风险
- SEC-013: strncpy使用模式
- SEC-014: Windows setenv行为差异
- SEC-015: 字符串字面量转义解析
- SEC-016: 内存泄漏

### 代码质量 (7个)
- LOGIC-012: check_only路径跳过清理
- LOGIC-013: 不可达代码
- LOGIC-014: 未使用的函数参数
- LOGIC-015: errors_found变量从未递增
- LOGIC-016: 不一致的平台宏
- LOGIC-017: 启发式字符串变量检测
- LOGIC-018: yyparse失败未传播

### 文档问题 (6个)
- DOC-007: 重复的LearnVix和VixDocs条目
- DOC-008: docs/COMPILER.md引用不存在的文件
- DOC-009: str关键字已移除但仍列出
- DOC-010: 过时的函数语法
- DOC-011: 重复的Testing部分
- DOC-012: Unix专用路径

### 构建系统 (5个)
- BUILD-007: 废弃的CMake命令
- BUILD-008: LLVM版本不匹配
- BUILD-009: 未固定的CI包版本
- BUILD-010: pyproject.toml使用遗留后端
- BUILD-011: 缺少测试依赖声明

### 测试覆盖 (4个)
- TEST-007: CLI标志未测试
- TEST-008: 薄弱断言
- TEST-009: 示例测试不完整
- TEST-010: 重复测试

---

## P3 - 1个月内修复 (24个)

### 安全漏洞 (4个)
- SEC-017: gets()函数支持
- SEC-018: 硬编码系统库路径
- SEC-019: printf格式字符串解析
- SEC-020: 源代码行长度验证缺失

### 代码质量 (5个)
- LOGIC-019: inline_imports返回值被丢弃
- LOGIC-020: 静态缓冲区线程安全问题
- LOGIC-021: fread返回值未检查
- LOGIC-022: YYLTYPE重新定义
- LOGIC-023: 输入文件未关闭

### 文档问题 (3个)
- DOC-013: 网站Getting Started链接404
- DOC-014: usize矛盾
- DOC-015: README-zh_CN.md缺少Ecosystem链接

### 构建系统 (5个)
- BUILD-012: 缺少CMake预设
- BUILD-013: 硬编码的安装目标
- BUILD-014: 未记录的VIX_DEBUG环境变量
- BUILD-015: .gitignore重复项
- BUILD-016: 构建工件已提交

### 测试覆盖 (7个)
- TEST-011: 弱断言 - not None
- TEST-012: 不一致的断言风格
- TEST-013: Unix专用测试路径
- TEST-014: 测试配置过于宽泛
- TEST-015: 缺少测试文档
- TEST-016: 无单元测试
- TEST-017: 无快照测试

---

## 统计总览

```
P0 ████████ 8 (7%)
P1 ████████████████████████████ 28 (25%)
P2 ███████████████████████████████████ 29 (26%)
P3 ██████████████████████████████████████████████ 24 (22%)
P4 ██████████ 6 (5%)
P5 ████████ 4 (4%)
   未分类 █████████████████████ 21 (19%)
```

**总计**: 110个问题

---

## 按类别分布

| 类别 | 数量 | 占比 |
|------|------|------|
| 🔒 安全漏洞 | 20 | 18% |
| 🐛 代码质量 | 24 | 22% |
| 📝 文档问题 | 15 | 14% |
| 🔧 构建系统 | 16 | 15% |
| 🧪 测试覆盖 | 18 | 16% |
| **总计** | **93** | **100%** |
