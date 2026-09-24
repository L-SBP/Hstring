#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "hpp/Hstring.hpp"

using String = Hstring<>;

static int g_failures = 0;

// 自制的检查宏：失败时继续跑完所有用例并统计，而不是在第一个 assert 就中止。
#define CHECK(cond)                                                           \
    do {                                                                      \
        if(!(cond)) {                                                         \
            std::cout << "FAILED line " << __LINE__ << ": " << #cond << '\n'; \
            ++g_failures;                                                     \
        }                                                                     \
    } while(false)

static void section(const char* name) {
    std::cout << "--- " << name << '\n';
}

// 校验内容、长度、终止符与空状态。
static void expect(const String& value, const char* expected) {
    const String::size_type n = std::char_traits<char>::length(expected);
    CHECK(value.size() == n);
    CHECK(value.length() == n);
    CHECK(std::string(value.c_str()) == expected);
    if(n > 0) {
        CHECK(!value.empty());
        CHECK(std::string(value.data(), n) == expected);
    } else {
        CHECK(value.empty());
    }
}

template<typename Fn>
static void expect_throws(Fn fn) {
    bool threw = false;
    try {
        fn();
    } catch(const std::out_of_range&) {
        threw = true;
    }
    CHECK(threw);
}

int main() {
    // ---------------- 构造函数 ----------------
    section("constructors");

    String empty;                        // Hstring()
    expect(empty, "");
    CHECK(empty.capacity() == 0);
    CHECK(empty.data() == nullptr);

    String from_cstr("hello");           // Hstring(const char*)
    expect(from_cstr, "hello");

    String null_string(nullptr);         // nullptr 防御：视作空串
    expect(null_string, "");

    expect(String("", 0), "");           // Hstring(const char*, size_type)
    expect(String("hello", 0), "");
    expect(String("hello", 2), "he");
    expect(String("hello", 5), "hello");
    expect(String("hello", 20), "hello");   // count 超过源串长度，不得越界读

    expect(String(4, 'x'), "xxxx");      // Hstring(size_type, char)
    expect(String(0, 'x'), "");
    String with_nul(1, '\0');            // 内容就是 '\0'，长度为 1
    CHECK(with_nul.size() == 1);

    String listed{'a', 'b', 'c'};        // Hstring(std::initializer_list<char>)
    expect(listed, "abc");
    expect(String{}, "");

    const char source[] = "abcdef";      // Hstring(InputIt, InputIt) 指针
    expect(String(source, source + 3), "abc");
    expect(String(source, source), "");

    std::string std_str = "wxyz";        // Hstring(InputIt, InputIt) 容器迭代器
    expect(String(std_str.begin(), std_str.end()), "wxyz");

    std::istringstream iss("single-pass");   // 真正的单遍输入迭代器
    expect(String(std::istreambuf_iterator<char>(iss),
                  std::istreambuf_iterator<char>()),
           "single-pass");

    // ---------------- 拷贝构造 / 移动构造 ----------------
    section("copy and move constructors");

    String copied(from_cstr);            // Hstring(const Hstring&)
    expect(copied, "hello");
    copied[0] = 'H';                     // 深拷贝：改副本不影响原对象
    expect(copied, "Hello");
    expect(from_cstr, "hello");

    String copy_of_empty(empty);         // 拷贝一个空对象
    expect(copy_of_empty, "");

    String moved(std::move(copied));     // Hstring(Hstring&&)
    expect(moved, "Hello");
    expect(copied, "");                  // 被移动对象处于有效空状态
    CHECK(copied.data() == nullptr);

    String moved_empty(std::move(copy_of_empty));
    expect(moved_empty, "");

    // ---------------- 赋值运算符 ----------------
    section("assignment operators");

    String copy_assigned;                // 目标无缓冲区
    copy_assigned = from_cstr;           // operator=(const Hstring&)
    expect(copy_assigned, "hello");
    copy_assigned = copy_assigned;       // 自赋值
    expect(copy_assigned, "hello");

    String empty_src;
    copy_assigned = empty_src;           // 从空对象拷贝赋值
    expect(copy_assigned, "");

    String move_assigned;                // operator=(Hstring&&)
    move_assigned = std::move(moved);
    expect(move_assigned, "Hello");
    expect(moved, "");
    String& self_move_ref = move_assigned;
    move_assigned = std::move(self_move_ref);   // 自移动赋值（经引用绕开 -Wself-move）
    expect(move_assigned, "Hello");

    String str_assigned("long value");   // operator=(const char*)
    str_assigned = "short";              // 复用已有容量
    expect(str_assigned, "short");
    str_assigned = "a much longer value";    // 需要重新分配
    expect(str_assigned, "a much longer value");
    str_assigned = "";                   // 赋空串
    expect(str_assigned, "");

    String str_assign_to_empty;
    str_assign_to_empty = "fill";        // 目标无缓冲区
    expect(str_assign_to_empty, "fill");
    str_assign_to_empty = str_assign_to_empty.c_str();   // 自别名
    expect(str_assign_to_empty, "fill");

    String char_assigned("abc");         // operator=(char)
    char_assigned = 'Z';
    expect(char_assigned, "Z");

    String char_assign_to_empty;         // 目标无缓冲区
    char_assign_to_empty = 'Q';
    expect(char_assign_to_empty, "Q");

    String char_assign_one_slot("");     // 容量恰好为 1，写第 2 个字节需扩容
    CHECK(char_assign_one_slot.capacity() == 1);
    char_assign_one_slot = 'W';
    expect(char_assign_one_slot, "W");

    String init_assigned("abc");         // operator=(std::initializer_list<char>)
    init_assigned = {'u', 'v', 'w'};
    expect(init_assigned, "uvw");
    init_assigned = {};                  // 空列表
    expect(init_assigned, "");

    String init_assign_to_empty;         // 目标无缓冲区
    init_assign_to_empty = {'x', 'y'};
    expect(init_assign_to_empty, "xy");

    // ---------------- 观察器 ----------------
    section("observers");

    String obs("abc");
    CHECK(obs.size() == 3);
    CHECK(obs.length() == 3);
    CHECK(!obs.empty());
    CHECK(obs.capacity() >= obs.size() + 1);
    CHECK(obs.max_size() > 1000);
    CHECK(String::npos == static_cast<String::size_type>(-1));

    String obs_empty;
    CHECK(obs_empty.size() == 0);
    CHECK(obs_empty.length() == 0);
    CHECK(obs_empty.empty());

    // ---------------- 容量操作 ----------------
    section("reserve / shrink_to_fit / resize / clear");

    String r("hello");
    r.reserve(100);                      // reserve
    CHECK(r.capacity() >= 100);
    expect(r, "hello");                  // 内容必须保留
    r.reserve(4);                        // 小于当前容量：不做任何事
    CHECK(r.capacity() >= 100);
    expect(r, "hello");

    String r_empty;                      // 无缓冲区上 reserve
    r_empty.reserve(8);
    CHECK(r_empty.capacity() >= 8);
    CHECK(r_empty.empty());
    expect(r_empty, "");

    String sh("hello");                  // shrink_to_fit
    sh.reserve(100);
    sh.shrink_to_fit();
    CHECK(sh.capacity() == sh.size() + 1);
    expect(sh, "hello");

    String sh_empty;                     // 无缓冲区：直接返回
    sh_empty.shrink_to_fit();
    CHECK(sh_empty.capacity() == 0);

    String sh_reserved_empty;
    sh_reserved_empty.reserve(16);
    sh_reserved_empty.shrink_to_fit();
    CHECK(sh_reserved_empty.capacity() == 1);
    expect(sh_reserved_empty, "");

    String rs("hello");
    rs.resize(2);                        // resize(count) 缩短
    CHECK(rs.size() == 2);
    expect(rs, "he");

    rs.resize(5, 'z');                   // resize(count, ch) 增长并填充
    CHECK(rs.size() == 5);
    expect(rs, "hezzz");

    rs.resize(5);                        // 长度不变
    expect(rs, "hezzz");

    rs.resize(8);                        // 默认用 '\0' 填充
    CHECK(rs.size() == 8);
    CHECK(std::string(rs.data(), rs.size()) ==
          std::string("hezzz") + std::string(3, '\0'));

    rs.resize(10, 'k');                  // 再次增长，触发扩容
    CHECK(rs.size() == 10);
    CHECK(rs.capacity() >= 11);

    rs.resize(0);                        // 缩到空
    CHECK(rs.empty());
    expect(rs, "");

    String rs_empty;                     // 无缓冲区上 resize
    rs_empty.resize(3, 'q');
    expect(rs_empty, "qqq");

    String cl("hello");                  // clear
    cl.reserve(64);
    cl.clear();
    CHECK(cl.empty());
    CHECK(cl.size() == 0);
    CHECK(cl.capacity() == 0);
    CHECK(cl.data() == nullptr);
    expect(cl, "");                      // c_str() 仍返回有效空串
    cl = "reuse";                        // clear 之后仍可复用
    expect(cl, "reuse");

    // ---------------- 元素访问 ----------------
    section("element access");

    String acc("abc");
    acc[0] = 'A';                        // operator[] 非 const 写
    acc[2] = 'C';
    CHECK(acc[1] == 'b');                // operator[] 非 const 读
    expect(acc, "AbC");

    const String& cacc = acc;            // operator[] const
    CHECK(cacc[0] == 'A');
    CHECK(cacc[2] == 'C');

    CHECK(acc.at(1) == 'b');             // at 非 const
    acc.at(1) = 'B';
    expect(acc, "ABC");
    CHECK(cacc.at(0) == 'A');            // at const
    expect_throws([&]{ (void)cacc.at(3); });     // 恰好等于 size()：越界
    expect_throws([&]{ (void)acc.at(100); });

    String acc_empty;
    expect_throws([&]{ (void)acc_empty.at(0); });

    acc.front() = 'x';                   // front 非 const
    expect(acc, "xBC");
    acc.back() = 'z';                    // back 非 const
    expect(acc, "xBz");
    CHECK(cacc.front() == 'x');          // front const
    CHECK(cacc.back() == 'z');           // back const
    expect_throws([&]{ (void)acc_empty.front(); });
    expect_throws([&]{ (void)acc_empty.back(); });

    char* raw = acc.data();              // data() 非 const
    CHECK(raw != nullptr);
    CHECK(raw[0] == 'x');
    raw[1] = 'Y';
    expect(acc, "xYz");

    const String& cdata = acc;           // data() const
    CHECK(cdata.data() != nullptr);
    CHECK(cdata.data()[1] == 'Y');

    CHECK(std::string(acc.c_str()) == "xYz");    // c_str()
    CHECK(acc.c_str()[3] == '\0');               // 保证以 '\0' 结尾
    CHECK(std::string(String().c_str()) == "");  // 空对象也要返回有效指针

    // ---------------- 迭代器 ----------------
    section("iterators");

    String it("abc");
    CHECK(std::string(it.begin(), it.end()) == "abc");        // begin/end
    CHECK(it.end() - it.begin() == 3);
    CHECK(*it.begin() == 'a');
    CHECK(*(it.end() - 1) == 'c');
    *it.begin() = 'X';                                        // 迭代器可写
    CHECK(*it.begin() == 'X');
    CHECK(std::string(it.begin(), it.end()) == "Xbc");

    const String& cit = it;                                   // const begin/end
    CHECK(std::string(cit.begin(), cit.end()) == "Xbc");
    CHECK(*cit.begin() == 'X');

    CHECK(std::string(it.cbegin(), it.cend()) == "Xbc");      // cbegin/cend
    CHECK(it.cbegin() == it.begin());

    CHECK(std::string(it.rbegin(), it.rend()) == "cbX");      // rbegin/rend
    CHECK(*it.rbegin() == 'c');
    CHECK(*(it.rend() - 1) == 'X');
    *it.rbegin() = 'C';                                       // 反向迭代器可写
    CHECK(std::string(it.begin(), it.end()) == "XbC");

    CHECK(std::string(cit.rbegin(), cit.rend()) == "CbX");    // const rbegin/rend
    CHECK(std::string(it.crbegin(), it.crend()) == "CbX");    // crbegin/crend

    String it_empty;                                          // 空对象：区间必须为空
    CHECK(it_empty.begin() == it_empty.end());
    CHECK(it_empty.cbegin() == it_empty.cend());
    CHECK(it_empty.rbegin() == it_empty.rend());
    CHECK(it_empty.crbegin() == it_empty.crend());
    CHECK(std::string(it_empty.begin(), it_empty.end()) == "");
    CHECK(it_empty.begin() == nullptr);                        // 未分配时为 nullptr
    CHECK(it_empty.end() == nullptr);
    CHECK(it_empty.end() - it_empty.begin() == 0);

    // 迭代器作为输入区间，直接喂给迭代器构造函数。
    String it_roundtrip(it.begin(), it.end());
    expect(it_roundtrip, "XbC");
    String it_reversed(it.rbegin(), it.rend());
    expect(it_reversed, "CbX");

    // 范围 for 循环依赖 begin/end。
    std::string collected;
    for(char ch : it) {
        collected.push_back(ch);
    }
    CHECK(collected == "XbC");

    // ---------------- 析构函数 ----------------
    section("destructor");

    {                                    // 作用域结束触发析构
        String scoped("scoped");
        expect(scoped, "scoped");
        String scoped_copy(scoped);
        String scoped_moved(std::move(scoped_copy));
        expect(scoped_moved, "scoped");
    }
    { String scoped_empty; }
    { String scoped_null(nullptr); }
    { String scoped_tiny(1, 'a'); }

    std::cout << (g_failures == 0 ? "All Hstring tests passed.\n"
                                  : "SOME TESTS FAILED\n");
    return g_failures == 0 ? 0 : 1;
}
