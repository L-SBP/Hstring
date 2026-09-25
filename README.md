# Hstring

一个从零实现的 `std::string` 风格动态字符串类，用于学习模板、分配器、迭代器和 C++ 对象生命周期（三/五法则）。

接口设计对齐 `std::string`，支持自定义分配器、完整迭代器、容量管理与查找操作。

```cpp
#include "hpp/Hstring.hpp"
#include <iostream>

int main() {
    Hstring s("hello");                      // Hstring<> == Hstring<std::allocator<char>>
    s += " world";                           // operator+=(const char*)
    s.push_back('!');                        // 单字符追加用 push_back（没有 append(char) 重载）
    s.replace(0, 5, "HELLO");                // 原地替换 [0,5) 区间

    for(char c : s)  std::cout << c;         // 范围 for：依赖 begin/end
    std::cout << '\n';                       // 输出：HELLO world!

    // 查找失败返回 npos
    return s.find("world") == Hstring<>::npos ? 1 : 0;
}
```

---

## 目录结构

```
Hstring/
├── CMakeLists.txt      # 构建配置
├── hpp/
│   └── Hstring.hpp     # 类声明 + 成员模板 + 非成员运算符定义
├── cpp/
│   ├── Hstring.cpp     # 非模板成员函数定义 + 显式实例化
│   └── main.cpp        # 测试套件（334 个断言，22 个分组）
└── build/              # 构建输出（已 gitignore）
```

---

## 构建与运行

依赖：CMake ≥ 3.0、支持 C++17 的编译器（GCC 13 / Clang / MSVC 均可）。

```bash
mkdir -p build && cd build
cmake ..
make
./Hstring
```

全部通过时输出：

```
--- constructors
--- copy and move constructors
...
--- stream input / output
All Hstring tests passed.
```

测试失败会打印 `FAILED` 行并返回非零退出码，便于接入 CI。

### 带检查的构建（推荐）

```bash
cd build
g++ -std=c++17 -fsanitize=address,undefined -g \
    -Wall -Wextra -Wpedantic \
    -I.. ../cpp/main.cpp ../cpp/Hstring.cpp -o Hstring_asan
./Hstring_asan
```

该配置下当前为**零警告、零内存错误、零泄漏、零 UB**。

---

## 类模板

```cpp
template<typename Allocator = std::allocator<char>>
class Hstring;
```

* `Hstring<>` 等价于 `Hstring<std::allocator<char>>`，日常使用直接写 `Hstring`。
* 存储来自 `Allocator::allocate`，自定义分配器只需满足标准 Allocator 要求。

**类型别名**：`value_type`、`size_type`、`difference_type`、`reference`、`const_reference`、`pointer`、`const_pointer`、`iterator`、`const_iterator`、`reverse_iterator`、`const_reverse_iterator`。

**静态常量**：`npos`（`size_type(-1)`）。

---

## API 一览

<details open>
<summary><b>构造 / 析构 / 赋值</b></summary>

| 接口 | 说明 |
| --- | --- |
| `Hstring()` | 空串，不分配内存 |
| `Hstring(const char* s)` | 从 C 字符串构造；`nullptr` 视为空串 |
| `Hstring(const char* s, size_type count)` | 取前 `min(count, strlen(s))` 个字符 |
| `Hstring(size_type count, char ch)` | `count` 个 `ch` |
| `Hstring(const Hstring&)` | 拷贝构造（深拷贝） |
| `Hstring(Hstring&&) noexcept` | 移动构造（O(1) 窃取指针） |
| `Hstring(InputIt first, InputIt last)` | 迭代器区间，支持单遍输入迭代器 |
| `Hstring(std::initializer_list<char>)` | `Hstring{'a','b'}` |
| `~Hstring()` | 释放缓冲区 |
| `operator=(const Hstring&)` | 拷贝赋值（copy-and-swap，自赋值安全、异常安全） |
| `operator=(Hstring&&) noexcept` | 移动赋值 |
| `operator=(const char*)` | 支持自别名 `s = s.c_str()` |
| `operator=(char)` | 单字符赋值 |
| `operator=(std::initializer_list<char>)` | 列表赋值 |

</details>

<details>
<summary><b>观察器</b></summary>

| 接口 | 说明 |
| --- | --- |
| `size()` / `length()` | 字符数（不含结尾 `'\0'`） |
| `max_size()` | 理论上限 |
| `capacity()` | 已分配槽位数（**含**结尾 `'\0'`） |
| `empty()` | `size() == 0` |

</details>

<details>
<summary><b>容量操作</b></summary>

| 接口 | 说明 |
| --- | --- |
| `reserve(new_cap)` | 扩容到至少 `new_cap`；小于当前容量时不做任何事 |
| `shrink_to_fit()` | 收缩到 `size() + 1` |
| `resize(count)` | 调整长度，新增部分填 `'\0'` |
| `resize(count, ch)` | 调整长度，新增部分填 `ch` |
| `clear()` | 清空并释放缓冲区 |

</details>

<details>
<summary><b>元素访问</b></summary>

| 接口 | 说明 |
| --- | --- |
| `operator[](idx)` | 无边界检查（const / 非 const） |
| `at(idx)` | 越界抛 `std::out_of_range` |
| `front()` / `back()` | 首/尾字符；空串抛 `std::out_of_range` |
| `data()` | 原始缓冲区指针 |
| `c_str()` | 保证 `'\0'` 结尾的只读指针（空对象返回 `""`） |

</details>

<details>
<summary><b>迭代器</b></summary>

`begin` / `end` / `cbegin` / `cend` / `rbegin` / `rend` / `crbegin` / `crend`，
每个都有 const 与非 const 版本，可用于范围 `for` 和 STL 算法。

</details>

<details>
<summary><b>修改操作</b></summary>

| 接口 | 说明 |
| --- | --- |
| `operator+=` | 追加 `Hstring` / `const char*` / `char` / 初始化列表 |
| `append(...)` | 5 个重载 + 迭代器区间模板 |
| `push_back(ch)` / `pop_back()` | 尾部增删 |
| `assign(...)` | 4 个重载 + 迭代器区间模板 |
| `insert(pos, ...)` | 3 个下标重载 + 2 个迭代器重载 |
| `erase(...)` | 下标区间 / 单迭代器 / 迭代器区间 |
| `replace(pos, count, ...)` | 3 个重载 |
| `swap(other)` / `swap(a, b)` | 成员与非成员（ADL） |

</details>

<details>
<summary><b>子串与比较</b></summary>

| 接口 | 说明 |
| --- | --- |
| `substr(pos, count)` | 取子串 |
| `copy(dest, count, pos)` | 拷贝到外部缓冲区，返回实际拷贝数 |
| `compare(...)` | 3 个重载，返回 `<0` / `0` / `>0` |

</details>

<details>
<summary><b>查找（每个函数 3 个重载：Hstring / const char* / char）</b></summary>

| 接口 | 说明 |
| --- | --- |
| `find` | 首次出现位置 |
| `rfind` | 末次出现位置 |
| `find_first_of` / `find_last_of` | 任一字符的出现位置 |
| `find_first_not_of` / `find_last_not_of` | 不属于给定字符集的位置 |

未找到时返回 `npos`。

</details>

<details>
<summary><b>非成员运算符</b></summary>

| 接口 | 说明 |
| --- | --- |
| `==` `!=` | 支持 `Hstring` 与 `const char*` 双向混用 |
| `<` `<=` `>` `>=` | `(Hstring, Hstring)` |
| `+` | 5 个重载：`Hstring+Hstring`、`+const char*`、`const char*+`、`+char`、`char+` |
| `<<` / `>>` | 流输出 / 流输入（`>>` 跳过前导空白） |
| `getline(is, str [, delim])` | 按分隔符读一行 |
| `swap(a, b)` | 非成员交换 |

</details>

---

## 设计要点

### 空串的表示

空串统一表示为 `data_ == nullptr, size_ == 0, capacity_ == 0`，**不分配缓冲区**。

由此带来的约定：

* `begin() == end() == nullptr`（合法空区间，可用于范围 `for` 和 STL 算法）
* `c_str()` 在空对象上返回静态的 `""`，而非 `nullptr`
* 对 `nullptr` 做指针相减是 UB，所以成员内部用私有辅助函数 `index_of(it)` 统一处理

### 容量约定

`capacity_` 计入结尾的 `'\0'`，因此刚构造出的串满足 `capacity() == size() + 1`。
`allocate` 与 `deallocate` 的元素个数必须一致，修改容量相关代码时请保持该约定。

### 模板代码的组织方式

| 成员类型 | 参数是否开放 | 定义位置 | 原因 |
| --- | --- | --- | --- |
| 普通成员（仅依赖类模板参数 `Allocator`） | 否 | `cpp/Hstring.cpp` + 末尾显式实例化 | 可预先列出所有特化 |
| 成员模板（如 `InputIt`） | 是，无法穷举 | 头文件 | 随用随实例化 |

`cpp/Hstring.cpp` 末尾的 `template class Hstring<std::allocator<char>>;` 必不可少：
**模板定义本身不产生机器码，只有实例化才会**。去掉这行，`Hstring.o` 将不含任何 `Hstring` 符号，链接阶段报 `undefined reference`。

代价：`Hstring<std::pmr::polymorphic_allocator<char>>` 这类未列出的特化会链接失败。
若需要支持任意分配器，改成 header-only，或在头文件加 `extern template` 声明。

### 自别名处理

以下写法中源数据就是对象自己的缓冲区，一旦扩容源指针即失效：

```cpp
s.append(s.c_str());
s.insert(0, s);
s.replace(1, 2, s);
```

实现通过 `points_into()` 判断源指针是否落在自身缓冲区内，命中则先复制到临时对象再重试。

### 单遍输入迭代器

`Hstring(InputIt, InputIt)`、`append`、`assign` 的迭代器版本采用**单遍遍历 + 容量倍增**，
不使用 `std::distance` 预先测量长度——那会消耗掉 `istreambuf_iterator` 这类单遍迭代器。
因此下列用法是安全的：

```cpp
std::istringstream iss("hello");
Hstring s(std::istreambuf_iterator<char>(iss), std::istreambuf_iterator<char>());
```

### 异常安全

* 拷贝赋值采用 copy-and-swap：先拷贝成功再交换，失败时原对象不受影响
* 所有扩容通过 `reserve()` 完成：先分配新缓冲区，拷贝成功后才释放旧的
* 迭代器区间构造在扩容过程中若抛异常，已分配的内存会被正确释放

---

## 与 `std::string` 的差异

| 项目 | `Hstring` | `std::string` |
| --- | --- | --- |
| `Hstring(const char* s, count)` | 钳制到 `min(count, strlen(s))`，更安全 | 严格读 `count` 字节，`count > strlen(s)` 是 UB |
| SSO（小字符串优化） | 未实现，空串恒为 `nullptr` | 通常有 |
| 迭代器类型 | 裸指针 `char*` | 类类型（`__normal_iterator`） |
| 关系运算符与 C 字符串混用 | `<` `<=` `>` `>=` 仅支持 `(Hstring, Hstring)` | 有 `(basic_string, const charT*)` 重载，可直接混用 |

> 由于 `Hstring(const char*, count)` 会按 `strlen` 钳制，**无法**用它嵌入 `'\0'`。
> 需要嵌入 `'\0'` 时请用 `assign(const char*, count)` 或迭代器区间构造，例如
> `s.assign("a\0b", 3)`。

### 已知待改进项

1. **迭代器是裸指针带来的重载歧义。** 字面量 `0` 既是整数又是空指针常量：

   ```cpp
   s.erase(0, 0);        // 歧义：(size_type, size_type) vs (const_iterator, const_iterator)
   s.insert(0, 3, '#');  // 歧义：(size_type, size_type, char) vs (const_iterator, size_type, char)
   ```

   暂时用显式类型规避：`const Hstring<>::size_type zero = 0; s.erase(zero, zero);`
   根本解法是把迭代器包装成类类型。

2. **`Allocator` 未作为成员存储。** 目前每个构造函数/析构函数内临时构造一个分配器对象，
   对 `std::allocator` 这类无状态分配器正确；若换用有状态分配器（内存池、`pmr`），
   `allocate` 与 `deallocate` 可能不是同一个对象，属于 UB。
   修法：加成员 `[[no_unique_address]] Allocator alloc_;`。

3. **未实现 SSO**，短字符串也会堆分配。

---

## 测试

`cpp/main.cpp` 是自包含的测试套件，共 **334 个断言、22 个分组**，覆盖全部公开接口：

| 分组 | 内容 |
| --- | --- |
| constructors | 8 个构造函数，含 `nullptr`、`count` 超长、单遍输入迭代器 |
| copy and move constructors | 深拷贝验证、移动后源对象为空 |
| assignment operators | 5 个重载，含自赋值、自移动、自别名、空缓冲区目标 |
| observers | `size` `length` `max_size` `capacity` `empty` `npos` |
| reserve / shrink_to_fit / resize / clear | 含无缓冲区对象上的调用 |
| element access | 各访问接口的 const / 非 const 版本与越界异常 |
| iterators | 正向、反向、const 版本、空对象区间、范围 `for` |
| destructor | 各类对象的作用域退出 |
| operator+= | 4 个重载 |
| append | 5 个重载 + 迭代器模板 + 500 次扩容压力 |
| push_back / pop_back | 含反复扩容与空串上的调用 |
| assign | 4 个重载 + 迭代器模板 |
| insert | 5 个重载，含空串插入、自插入 |
| erase | 3 个重载 |
| replace | 3 个重载，含变长/变短/自替换 |
| swap | 成员与非成员、与空对象交换、自交换 |
| substr and copy | `pos == size()` 等边界 |
| compare | 3 个重载 |
| find / rfind / find_*_of | 18 个函数 |
| comparison operators | 6 个运算符，含 `const char*` 混用 |
| operator+ | 5 个重载 |
| stream input / output | `<<` `>>` `getline`×2，含嵌入 `'\0'` 的内容 |

测试策略：**以 `std::string` 作为参照实现（oracle）**。
`find` / `rfind` / `compare` / `insert` / `erase` / `replace` / `append` / `copy` / `substr`
等工作在「多组文本 × 多组 needle × 多个位置」的嵌套循环中与 `std::string` 逐字节对比，
比手算期望值可靠。内容比对一律走 `data() + size()`，不用 `c_str()`——内容可能含嵌入 `'\0'`。

---

## 许可

学习用途项目，可自由使用与修改。
