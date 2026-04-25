# clang-format 配置

基于 Linux 内核风格，大括号换行，空格缩进，snake_case 命名。

---

## BasedOnStyle: LLVM

以 LLVM 为基础风格，再逐项覆盖调整。

---

### IndentWidth: 4

每级缩进 4 个空格。

```cpp
// 格式化后
void foo()
{
    if (x) {
        bar();
    }
}
```

---

### TabWidth: 4 / UseTab: Never

Tab 视觉宽度为 4，但实际全部输出空格，保证不同编辑器渲染一致。

---

### ColumnLimit: 80

每行不超过 80 个字符，超出时自动换行。

```cpp
// 超出 80 字符时自动断行
void very_long_function_name(
        int first_param,
        int second_param,
        int third_param);
```

---

### BreakBeforeBraces: Linux

函数/类/命名空间的左大括号换行，控制语句的左大括号不换行。

```cpp
void foo()          // 函数：换行
{
    if (condition) { // 控制语句：不换行
        bar();
    }
}
```

---

### PointerAlignment: Right / PointerBindsToType: false

指针和引用符号靠近变量名。

```cpp
int *ptr;      // ✓
int &ref;      // ✓
int* ptr;      // ✗
```

---

### SpaceBeforeParens: ControlStatements

仅在控制语句（`if`/`for`/`while`/`switch`）前加空格，函数调用不加。

```cpp
if (x) { }     // ✓ 控制语句加空格
func(x);       // ✓ 函数调用不加空格
```

---

### SpacesInAngles: Never

模板角括号内不加空格。

```cpp
std::vector<int>      // ✓
std::vector< int >    // ✗
```

---

### SpacesInParentheses: false / SpacesInSquareBrackets: false

括号和方括号内不加空格。

```cpp
func(a, b);   // ✓
arr[0];       // ✓
func( a, b ); // ✗
arr[ 0 ];     // ✗
```

---

### SpaceAfterCStyleCast: false

C 风格强制转换后不加空格。

```cpp
(int)x    // ✓
(int) x   // ✗
```

---

### IndentCaseLabels: false

`case` 标签与 `switch` 同级，不额外缩进。

```cpp
switch (x) {
case 1:      // ✓ 与 switch 同级
    break;
}
```

---

### IndentPPDirectives: None

预处理指令顶格书写，不跟随代码缩进。

```cpp
#ifdef DEBUG      // ✓ 顶格
    #ifdef DEBUG  // ✗
```

---

### NamespaceIndentation: None

命名空间内容不缩进。

```cpp
namespace foo {
void bar();      // ✓ 不缩进
    void bar();  // ✗
}
```

---

### ContinuationIndentWidth: 8

换行续行时缩进 8 个空格（Linux 内核风格，与正文缩进区分）。

```cpp
int result = very_long_variable_name
        + another_long_name;  // 8 空格缩进
```

---

### AccessModifierOffset: -4

`public:`/`private:` 等访问修饰符相对类体缩进 -4，与 `class` 关键字对齐。

```cpp
class foo {
public:          // 与 class 同级
    void bar();
};
```

---

### BinPackParameters: false / BinPackArguments: false

参数要么全在一行，要么每个参数各占一行，禁止"能塞几个塞几个"的混排。

```cpp
// ✓ 全在一行
void foo(int a, int b, int c);

// ✓ 超过行宽时每个参数单独一行
void foo(
        int a,
        int b,
        int c);

// ✗ 禁止混排
void foo(int a, int b,
         int c);
```

---

### AlignAfterOpenBracket: BlockIndent

换行后参数以块缩进方式对齐（缩进 `ContinuationIndentWidth` 个空格），右括号单独成行。

```cpp
foo(
        arg1,
        arg2
);
```

---

### AllowShortFunctionsOnASingleLine: None

任何函数体都不允许单行。

```cpp
// ✓
int get()
{
    return x_;
}

// ✗
int get() { return x_; }
```

---

### AllowShortIfStatementsOnASingleLine: Never

`if` 语句体必须换行。

```cpp
// ✓
if (x)
    return;

// ✗
if (x) return;
```

---

### AllowShortLambdasOnASingleLine: Empty

只有空 lambda 可以单行。

```cpp
auto f = [] {};        // ✓ 空 lambda
auto g = [](int x) {  // ✓ 有内容则换行
    return x + 1;
};
```

---

### ReflowComments: false

不自动重新排版注释文字，保留手写换行。

---

### CommentPragmas: '^ IWYU pragma:|^ NOLINT'

以 `IWYU pragma:` 或 `NOLINT` 开头的注释视为特殊指令，不做格式化处理。

```cpp
#include <vector>  // NOLINT
```

---

### SortIncludes: CaseInsensitive / IncludeBlocks: Regroup

对 include 排序（大小写不敏感），并按 `IncludeCategories` 分组，组间插入空行。

```cpp
// 格式化后
#include "logger/logger.hpp"   // 组 1：项目内部

#include <spdlog/spdlog.h>     // 组 2：第三方库

#include <string>              // 组 3：系统库
#include <vector>
```

---

### IncludeCategories

| Priority | Regex | 含义 |
| -------- | ----- | ---- |
| 1 | `^"` | 项目内部头文件 |
| 2 | `^<.*(spdlog\|fmt\|boost\|eigen)` | 第三方库 |
| 3 | `^<` | 系统/标准库 |

---

### Cpp11BracedListStyle: false

关闭 C++11 初始化列表的紧凑风格，`{}` 内保留正常空格处理。

```cpp
int arr[] = {1, 2, 3};    // ✓
```

---

### BreakBeforeTernaryOperators: true

三元运算符 `?` 和 `:` 置于行首，便于快速识别条件分支。

```cpp
int x = condition
        ? true_value
        : false_value;
```

---

### AlwaysBreakTemplateDeclarations: Yes

模板声明 `template<...>` 始终单独一行。

```cpp
template <typename T>
void foo(T x);
```
