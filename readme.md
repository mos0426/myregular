# MyRegular

基于自动机理论的正则语言编译器（进行中）。

将正则表达式编译为 NFA / DFA，并在码点（UTF-8）级别上进行匹配。核心流水线为：

```text
正则表达式 → Lexer → Parser (AST) → Thompson 构造 (NFA) → 子集构造 (DFA) → 匹配
```

当前以静态库形式组织，尚无对外稳定的公共 API 与可执行入口；开发与验证主要通过单元测试完成。

## 当前进度

| 模块 | 状态 | 说明 |
|------|------|------|
| `utility` | 已完成 | UTF-8 码点解码 |
| `expression` | 已完成 | 词法分析、语法分析、AST、错误报告 |
| `automata` | 基本完成 | `CharSet`、`NFA`、`DFA`、`DFATransition` |
| `compile` | 基本完成 | AST→NFA（Thompson）、NFA→DFA（子集构造） |
| `interval` | 已完成 | 区间游标与归并器，准备替换编译路径中的区间代数实现 |
| DFA 最小化 | 未完成 | 头文件已声明 `dfa_minimization`，实现待补 |
| `{n,m}` 优化 | 规划中 | 当前为展开式构造，后续考虑子自动机 / 计数边等方案 |

## 架构

```text
include/internal/
├── utility/      # UTF-8 工具
├── expression/   # Lexer / Parser / AST
├── automata/     # CharSet、NFA、DFA
├── compile/      # AST → NFA → DFA
└── interval/     # 码点区间代数（新接口，集成中）
```

对应源码与测试分别在 `src/internal/`、`tests/internal/` 下按相同分层放置。

### 编译流水线

1. **Lexer**（`expression`）  
   将输入切分为原子、后缀算子、中缀算子、转义与结构控制符等 Token。

2. **Parser**（`expression`）  
   递归下降生成 AST，节点类型包括并、连接、`*` / `+` / `?`、`{n,m}`、单字符与字符类。

3. **compile_to_nfa**（`compile`）  
   基于 Thompson 构造，将 AST 编译为带 ε 转移的 NFA。字符与字符类边用码点区间（`CharSet`）表示。

4. **nfa_to_dfa**（`compile`）  
   基于子集构造，将 NFA 转为 DFA；边按码点区间合并。`minimize` 参数已预留，最小化尚未接入。

5. **匹配**  
   `NFA` / `DFA` 均提供 `consume(codepoint)` 与 `check()`，按码点消费输入并判断是否到达接受态。

### 区间与字符集

字符集与转移边以**左闭右开**的码点区间表示。`interval` 模块正在抽出通用的 `IntervalCursor` / `IntervalCursorMerger`，计划用于统一 `nfa_to_dfa` 等处的区间扫描与归并逻辑（见 `TODO.md`）。

约定：以 `*_set` 结尾、用 `std::vector` 表示的集合，在操作过程中保持**正序、无重复**。

## 已支持的正则语法

| 语法 | 含义 |
|------|------|
| `ab` | 连接 |
| `a\|b` | 并 |
| `a*` / `a+` / `a?` | 闭包 / 正闭包 / 可选（`?` 实现为 `{0,1}`） |
| `a{n}` / `a{n,}` / `a{n,m}` | 有限或无上界重复（展开构造，上限受 `MAX_REPETITION` 约束） |
| `.` | 任意码点 |
| `[abc]` / `[a-z]` / `[^...]` | 字符类 / 取反字符类 |
| `(...)` | 分组 |
| `\w` `\W` `\d` `\D` `\a` `\s` `\S` `\l` `\u` | 预定义字符类转义 |

输入按 UTF-8 解码为 Unicode 码点后再参与匹配。

## 构建与测试

依赖：

- CMake（项目要求见 `CMakeLists.txt`）
- C++20 编译器
- GoogleTest（通过 `FetchContent` 自动拉取）

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

测试目标包括：`expression_test`、`utf8_test`、`automata_test`、`compile_test`、`interval_test`。

示例（编译并匹配，摘自测试思路）：

```cpp
#include "compile.hpp"
#include "parse.hpp"

std::string_view sv = "[0-9]+\\.[0-9]+";
auto ast = parse(sv);
NFA nfa = compile_to_nfa(*ast);
DFA dfa = nfa_to_dfa(nfa);

for (char c : std::string("3.14")) {
    dfa.consume(static_cast<unsigned char>(c));
}
bool ok = dfa.check();  // true
```

## 下一步（摘自 `TODO.md`）

- dfa 最小化
    - 构建反向转移表
    - 划分等价状态集
- 测试修改成按照 utf-8 码点值输出
- dfa最小化
- RepetitionNode 编译及相对的 NFA, DFA机制的优化
    - 子自动机的方案
    - 计数边的方案


## 许可证

（待定）
