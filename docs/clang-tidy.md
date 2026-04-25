# clang-tidy 配置

基于 Linux 内核编码规范与现代 C++ 最佳实践。

---

## Checks

`Checks` 字段是一个以逗号分隔的规则列表，先用 `-*` 关闭所有内置检查，再逐条启用目标规则组。

---

### -*

关闭所有内置检查，从零开始按需启用，避免无关规则干扰。

---

### bugprone-*

检测常见编程错误：移动后使用、有符号/无符号转换、危险字符串操作等。

```cpp
// bugprone-use-after-move
auto v = std::move(vec);
vec.push_back(1);  // ✗ 移动后使用
```

---

### -bugprone-exception-escape

检测函数中是否存在异常逃逸风险，即函数内部可能抛出异常，但该异常未在函数内被捕获，可能向外传播。

该规则主要用于保证关键函数（如 main()、线程入口、回调函数等）不会出现未处理异常，从而导致程序 std::terminate() 或不符合工程异常安全约束。

```cpp
// main 未捕获异常
int main() {
    std::vector<int> v(1000000000); // 可能抛 std::bad_alloc
    return 0;
}

// STL 操作可能抛异常
void func() {
    std::map<int, int> m;
    m[1] = 2;  // 可能触发内存分配异常
}

// 调用未知异常来源函数
void run() {
    init_server(); // 内部可能 throw，但未处理
}

//推荐修复方式
//在函数边界统一捕获异常，避免逃逸：
int main()
try {
    run_app();
    return 0;
} catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
} catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 1;
}
```

---

### -bugprone-easily-swappable-parameters

连续同类型参数极常见，此规则误报率过高，禁用。

```cpp
// 以下代码会被此规则误报，但实际上是合理写法
void resize(int width, int height);       // ✗ 误报
void set_color(float r, float g, float b); // ✗ 误报
```

---

### -bugprone-implicit-widening-of-multiplication-result

`size_t` 与 `int` 混合计算时隐式提升频繁出现，误报率高，禁用。

```cpp
// 此规则会对以下常见写法误报
int rows = 100;
int cols = 200;
size_t total = rows * cols;  // ✗ 误报：int * int 隐式转 size_t
```

---

### cert-*

CERT C/C++ 安全编码标准：防止资源泄漏、不安全转换、危险函数调用等。

```cpp
// cert-err34-c：不安全的字符串转整数
int n = atoi(str);        // ✗
int n = strtol(str, nullptr, 10); // ✓
```

---

### -cert-err58-cpp

spdlog、fmt 等库的全局静态对象在初始化时可能抛出异常，此规则无法避免，禁用。

```cpp
// 第三方库的全局对象初始化，无法消除
static auto s_logger = spdlog::stdout_color_mt("main");  // ✗ 误报
```

---

### clang-analyzer-*

Clang 静态分析器：路径敏感分析，检测空指针解引用、内存泄漏、除零等。

```cpp
int *p = nullptr;
*p = 1;  // ✗ 空指针解引用

int x = 0;
int y = 10 / x;  // ✗ 除零
```

---

### cppcoreguidelines-*

C++ Core Guidelines：资源管理（RAII）、类型安全、禁用裸指针等。

```cpp
// cppcoreguidelines-owning-memory
void foo()
{
    int *p = new int(1);  // ✗ 裸 new，应使用智能指针
    auto p = std::make_unique<int>(1);  // ✓
}
```

---

### -cppcoreguidelines-avoid-c-arrays

系统级代码中 C 数组合理，禁用此规则。

```cpp
// 底层代码中 C 数组合理，不应强制替换
uint8_t buffer[256];      // ✓ 允许
char name[MAX_NAME_LEN];  // ✓ 允许
```

---

### -cppcoreguidelines-avoid-do-while

某些算法（如 Boyer-Moore）中 `do-while` 语义最清晰，禁用此规则。

```cpp
// do-while 语义清晰，不强制改写
do {
    process(chunk);
    chunk = read_next();
} while (chunk.valid());
```

---

### -cppcoreguidelines-macro-usage

日志宏、断言宏在项目中不可避免，禁用此规则。

```cpp
#define ASSERT(cond) ...   // ✓ 允许
#define LOG_INFO(msg) ...  // ✓ 允许
```

---

### -cppcoreguidelines-prefer-member-initializer

部分初始化逻辑依赖运行时计算，在构造函数体内更清晰，禁用此规则。

```cpp
class foo {
public:
    foo()
    {
        size_ = compute_size();  // ✓ 允许：运行时计算
    }
private:
    int size_;
};
```

---

### -cppcoreguidelines-pro-bounds-pointer-arithmetic

底层代码（如协议解析、内存映射）中指针运算合理，禁用此规则。

```cpp
// 底层解析中指针运算合理
uint8_t *ptr = buffer;
uint16_t id = *(ptr + 2);  // ✓ 允许
```

---

### -cppcoreguidelines-pro-type-vararg

封装 C 接口时 `printf` 系函数不可避免，禁用此规则。

```cpp
// C 接口封装中 vararg 合理
snprintf(buf, sizeof(buf), "%d-%d", major, minor);  // ✓ 允许
```

---

### misc-*

杂项检查：未使用参数、`using namespace` 全局污染、重复 `#include` 等。

```cpp
// misc-use-anonymous-namespace
static void helper() { }  // ✗ 文件内函数建议用匿名命名空间
namespace {
void helper() { }         // ✓
}
```

---

### -misc-include-cleaner

`spdlog` header-only 库的传递包含机制与此规则冲突，误报率高，禁用。

```cpp
// 只 include spdlog.h，但用到了其传递包含的类型
#include <spdlog/spdlog.h>
spdlog::info("hello");  // ✗ 误报：建议直接 include 内部头文件
```

---

### -misc-non-private-member-variables-in-classes

纯数据结构体（POD-like struct）的 public 成员合理，禁用此规则。

```cpp
struct point {
    float x;  // ✓ 允许：数据结构体 public 成员
    float y;
};
```

---

### modernize-*

推动代码向现代 C++ 迁移：`nullptr`、`override`、range-for、智能指针等。

```cpp
// modernize-use-nullptr
if (p == NULL)    // ✗
if (p == nullptr) // ✓

// modernize-use-override
class derived : public base {
    void foo();          // ✗
    void foo() override; // ✓
};
```

---

### -modernize-use-trailing-return-type

尾置返回类型仅在模板推导等必要场景使用，不应强制全局，禁用此规则。

```cpp
// 此规则会强制将所有函数改为尾置返回类型，过于激进
int get_count();           // ✓ 允许
auto get_count() -> int;   // ✗ 不强制要求
```

---

### performance-*

检测性能问题：不必要的拷贝、低效容器操作、冗余转换等。

```cpp
// performance-unnecessary-copy-initialization
std::string get_name();
std::string name = get_name();         // ✓ 可接受
const std::string &name = get_name();  // ✓ 更好：避免拷贝
```

---

### -performance-avoid-endl

`std::endl` 在非热路径中的强制替换收益极低且影响可读性，禁用。

```cpp
std::cout << "done" << std::endl;  // ✓ 允许
std::cout << "done" << '\n';       // ✓ 同样允许
```

---

### portability-*

检测移植性问题：平台相关的整数大小假设、不可移植的 SIMD intrinsic 等。

```cpp
// portability-simd-intrinsics：建议用标准算法替代平台特定 intrinsic
__m128i v = _mm_loadu_si128(...);  // ✗ x86-only，不可移植
```

---

### readability-*

提升代码可读性：命名规范、强制大括号、去除冗余转换、简化布尔表达式等。

```cpp
// readability-braces-around-statements
if (x)
    foo();  // ✗

if (x) {
    foo();  // ✓
}

// readability-redundant-casting
int x = (int)some_int;  // ✗ 冗余转换
```

---

### -readability-identifier-length

配合 `IgnoredParameterNames` 和 `IgnoredVariableNames` 已对短变量做了豁免，
全局启用此规则会对 `i`、`n` 等习惯用法报错，禁用。

```cpp
for (int i = 0; i < n; ++i) { }  // ✓ 允许：i、n 被豁免
```

---

## WarningsAsErrors: '*'

所有启用的检查项均视为编译错误，阻断构建。单行豁免用 `// NOLINT(check-name)`。

```cpp
int x = (int)y;  // NOLINT(cppcoreguidelines-narrowing-conversions)
```

---

## HeaderFilterRegex

只对匹配正则的头文件运行检查，忽略第三方库头文件。

```yaml
HeaderFilterRegex: '^.*(services|common)/.*\.(hpp|h)$'
```

项目头文件路径包含 `services/` 或 `common/`，spdlog/fmt 等第三方路径不匹配，不会被检查。

---

## SystemHeaders: false

不检查 `<vector>`、`<string>` 等系统头文件中的代码，防止第三方代码产生大量噪音。

---

## UseColor: true

终端彩色输出，错误/警告/提示用不同颜色区分，便于快速定位问题。

---

## CheckOptions

---

### readability-identifier-naming.VariableCase: lower_case

局部变量使用 `snake_case`。

```cpp
int buffer_size = 0;   // ✓
int bufferSize = 0;    // ✗
```

---

### readability-identifier-naming.ParameterCase: lower_case

函数参数使用 `snake_case`。

```cpp
void send(int max_retry, int timeout_ms);  // ✓
void send(int maxRetry, int timeoutMs);    // ✗
```

---

### readability-identifier-naming.ParameterIgnoredRegexp: '^[ijk_]$'

豁免参数名 `i`、`j`、`k`（循环变量习惯用法）和 `_`（占位符）不受命名约束。

```cpp
auto fn = [](int i, int _) { return i; };  // ✓ i 和 _ 被豁免
```

---

### readability-identifier-naming.FunctionCase: lower_case

函数名使用 `snake_case`。

```cpp
void init_logger();  // ✓
void InitLogger();   // ✗
```

---

### readability-identifier-naming.MemberCase: lower_case

类/结构体的成员变量使用 `snake_case`。

```cpp
struct header {
    uint16_t packet_id;   // ✓
    uint16_t packetId;    // ✗
};
```

---

### readability-identifier-naming.PrivateMemberCase: lower_case

私有成员变量使用 `snake_case`（配合 `PrivateMemberSuffix` 加尾部下划线）。

```cpp
class foo {
    int size_;   // ✓
    int Size_;   // ✗
};
```

---

### readability-identifier-naming.PrivateMemberSuffix: '_'

私有成员变量加 `_` 后缀，与局部变量一眼区分。

```cpp
class foo {
public:
    int get_size() const { return size_; }
private:
    int size_;   // ✓ 有后缀
    int size;    // ✗ 无后缀
};
```

---

### readability-identifier-naming.StructCase: lower_case

结构体名使用 `snake_case`。

```cpp
struct packet_header { };  // ✓
struct PacketHeader { };   // ✗
```

---

### readability-identifier-naming.ClassCase: lower_case

类名使用 `snake_case`。

```cpp
class tcp_socket { };  // ✓
class TcpSocket { };   // ✗
```

---

### readability-identifier-naming.NamespaceCase: lower_case

命名空间名使用 `snake_case`。

```cpp
namespace net { }      // ✓
namespace Net { }      // ✗
```

---

### readability-identifier-naming.TypeAliasCase: lower_case

`using` 类型别名使用 `snake_case`。

```cpp
using byte_t = uint8_t;   // ✓
using ByteT = uint8_t;    // ✗
```

---

### readability-identifier-naming.TypedefCase: lower_case

`typedef` 类型别名使用 `snake_case`。

```cpp
typedef uint8_t byte_t;   // ✓
typedef uint8_t ByteT;    // ✗
```

---

### readability-identifier-naming.ConstantCase: UPPER_CASE

`constexpr` 常量（非成员）使用全大写加下划线。

```cpp
constexpr int MAX_SIZE = 100;  // ✓
constexpr int maxSize = 100;   // ✗
```

---

### readability-identifier-naming.GlobalConstantCase: UPPER_CASE

全局 `const`/`constexpr` 常量使用全大写加下划线。

```cpp
constexpr int FILE_SIZE = 1048576;  // ✓
constexpr int file_size = 1048576;  // ✗
```

---

### readability-identifier-naming.EnumConstantCase: UPPER_CASE

枚举值使用全大写加下划线。

```cpp
enum class color {
    RED,    // ✓
    Green,  // ✗
};
```

---

### readability-identifier-naming.MacroDefinitionCase: UPPER_CASE

宏定义使用全大写加下划线。

```cpp
#define MAX_RETRY 3    // ✓
#define maxRetry 3     // ✗
```

---

### readability-simplify-boolean-expr.SimplifyDeMorgan: false

关闭 De Morgan 定律的自动化简，避免生成可读性更差的等价写法。

```cpp
if (!(a && b)) { }   // ✓ 保持原样
// 关闭后不会被改写为：
if (!a || !b) { }    // ✗ 不强制建议
```

---

### readability-identifier-length.IgnoredParameterNames: '^[ijk_]$'

允许参数名 `i`、`j`、`k`、`_` 不受最小标识符长度限制。

```cpp
void map(int i, int j) { }   // ✓ i、j 被豁免
```

---

### readability-identifier-length.IgnoredVariableNames: '^[ijk_]$'

允许变量名 `i`、`j`、`k`、`_` 不受最小标识符长度限制。

```cpp
for (int i = 0; i < n; ++i) { }  // ✓ i 被豁免
```

---

### bugprone-assert-side-effect.AssertMacros: 'ASSERT'

将项目自定义 `ASSERT` 宏注册为断言宏，避免对其参数中副作用的误报。

```cpp
ASSERT(counter++ > 0);  // ✓ 不报 "断言中有副作用"
```

---

### modernize-use-default-member-init.UseAssignment: '1'

成员默认初始化推荐赋值语法而非花括号语法。

```cpp
class foo {
    int count_ = 0;    // ✓ 推荐赋值语法
    int count_{0};     // ✗ 不建议花括号语法
};
```

---

### cppcoreguidelines-special-member-functions.AllowSoleDefaultDtor: '1'

允许类只声明 `= default` 析构函数而不实现完整五法则（Rule of Five），多态基类中常见。

```cpp
class base {
public:
    virtual ~base() = default;  // ✓ 不要求补充拷贝/移动构造函数
};
```
