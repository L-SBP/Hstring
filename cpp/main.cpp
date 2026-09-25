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

// 把 Hstring 转成 std::string，用于和标准库参照实现逐字节对比。
// 不能走 c_str()：内容里可能含内嵌 '\0'。
static std::string to_std(const String& s) {
    return s.size() == 0 ? std::string() : std::string(s.data(), s.size());
}

static void expect_std(const String& value, const std::string& expected) {
    if(to_std(value) != expected) {
        std::cout << "  expect_std FAILED: actual=[" << to_std(value)
                  << "] expected=[" << expected << "]\n";
        ++g_failures;
    }
}

// 校验内容、长度、终止符与空状态。失败时打印实际值，便于定位。
static void expect(const String& value, const char* expected) {
    const std::string exp(expected);
    if(to_std(value) != exp) {
        std::cout << "  expect FAILED: actual=[" << to_std(value)
                  << "] (size " << value.size() << "), expected=[" << exp
                  << "] (size " << exp.size() << ")\n";
        ++g_failures;
        return;
    }
    if(value.size() != value.length()) {
        std::cout << "  expect FAILED: size() != length()\n";
        ++g_failures;
    }
    if(value.c_str()[value.size()] != '\0') {
        std::cout << "  expect FAILED: missing null terminator\n";
        ++g_failures;
    }
    if(value.empty() != (value.size() == 0)) {
        std::cout << "  expect FAILED: empty() inconsistent\n";
        ++g_failures;
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

    // ---------------- operator+= ----------------
    section("operator+=");

    {
        String a("abc");
        CHECK(&(a += String("de")) == &a);      // operator+=(const Hstring&) 返回 *this
        expect(a, "abcde");

        String b("abc");
        b += "def";                             // operator+=(const char*)
        expect(b, "abcdef");

        String c("abc");
        c += '!';                               // operator+=(char)
        expect(c, "abc!");

        String d("abc");
        d += {'x', 'y'};                        // operator+=(initializer_list)
        expect(d, "abcxy");

        String e;                               // 空对象上混合追加（原实现漏 return 且泄漏）
        e += "hi";
        e += String("!");
        e += '?';
        e += {'~'};
        expect(e, "hi!?~");

        std::string o = "base";                 // 与 std::string 对照
        String      h("base");
        o += "1";          h += "1";
        o += '2';          h += '2';
        o.append(3, '3');  h.append(3, '3');
        o.append("tail");  h.append("tail");
        CHECK(to_std(h) == o);
    }

    // ---------------- append ----------------
    section("append");

    {
        String a("ab");
        CHECK(&a.append(String("cd")) == &a);    // append(const Hstring&)
        expect(a, "abcd");

        std::string o = "ab";
        String      h("ab");

        o.append("xxHELLOyy", 2, 5);   h.append(String("xxHELLOyy"), 2, 5);
        expect_std(h, o);                        // append(const Hstring&, pos, count)

        o.append("xxHELLOyy", 2, 100); h.append(String("xxHELLOyy"), 2, 100);
        expect_std(h, o);                        // count 超长应被截断，而不是越界

        o.append("abcdef", 3);         h.append("abcdef", 3);
        expect_std(h, o);                        // append(const char*, count)

        o.append("XYZ");               h.append("XYZ");
        expect_std(h, o);                        // append(const char*)

        o.append(3, '*');              h.append(3, '*');
        expect_std(h, o);                        // append(count, ch)

        const char chunk[] = "12";               // append(InputIt, InputIt) 模板
        o.append(chunk, chunk + 2);    h.append(chunk, chunk + 2);
        expect_std(h, o);

        std::istringstream iss("stream");        // 单遍输入迭代器也要正确
        String g("x");
        g.append(std::istreambuf_iterator<char>(iss),
                 std::istreambuf_iterator<char>());
        expect(g, "xstream");

        String big("z");                         // 多次扩容后仍保持内容与 '\0'
        for(int i = 0; i < 300; ++i)  big.push_back('.');
        big.append("end");
        CHECK(big.size() == 1 + 300 + 3);
        CHECK(big.data()[big.size()] == '\0');
        CHECK(big.data()[0] == 'z');

        expect_throws([&]{ String x("a"); x.append(String("b"), 5, 1); });
    }

    // ---------------- push_back / pop_back ----------------
    section("push_back / pop_back");

    {
        String s;
        s.push_back('a');                        // 空对象首次 push_back
        expect(s, "a");
        s.push_back('b');
        s.push_back('c');
        expect(s, "abc");
        CHECK(s.data()[3] == '\0');              // 始终以 '\0' 结尾

        for(int i = 0; i < 500; ++i)  s.push_back('x');   // 反复触发扩容
        CHECK(s.size() == 3 + 500);
        CHECK(s.capacity() >= s.size() + 1);
        CHECK(s.data()[s.size()] == '\0');
        CHECK(std::string(s.data(), 3) == "abc");        // 扩容不丢前缀

        String t = s;
        t.pop_back();
        CHECK(t.size() == s.size() - 1);
        CHECK(t.data()[t.size()] == '\0');

        String u("a");
        u.pop_back();                            // 删到空
        CHECK(u.empty());
        u.pop_back();                            // 空串上 pop_back 是安全空操作
        CHECK(u.empty());

        String v;
        v.pop_back();                            // 从未分配过的串上 pop_back
        CHECK(v.empty());
    }

    // ---------------- assign ----------------
    section("assign");

    {
        String s("initial");
        CHECK(&s.assign(String("hello")) == &s); // assign(const Hstring&)
        expect(s, "hello");

        s.assign("world");                       // assign(const char*)
        expect(s, "world");

        s.assign("abcdefgh", 3);                 // assign(const char*, count)
        expect(s, "abc");

        s.assign(4, 'z');                        // assign(count, ch)
        expect(s, "zzzz");

        s.assign(0, 'z');                        // count == 0 -> 清空
        CHECK(s.empty());

        std::string oracle = "alpha";            // assign(InputIt, InputIt) 模板
        s.assign(oracle.begin(), oracle.end());
        expect(s, "alpha");

        std::istringstream iss("single");        // 单遍输入迭代器
        s.assign(std::istreambuf_iterator<char>(iss),
                 std::istreambuf_iterator<char>());
        expect(s, "single");

        s.assign(s);                             // 自赋值
        expect(s, "single");

        s.assign(s.c_str());                     // 自别名：参数就是自己的缓冲区
        expect(s, "single");

        String e;                                // 空对象上 assign
        e.assign("fill");
        expect(e, "fill");
        e.assign(0, 'q');
        CHECK(e.empty());

        String wide("already long enough");       // 缩短赋值，复用已有容量
        wide.assign("s");
        expect(wide, "s");
    }

    // ---------------- insert ----------------
    section("insert");

    {
        String s("AC");
        CHECK(&s.insert(1, String("B")) == &s);  // insert(pos, const Hstring&)
        expect(s, "ABC");

        s.insert(0, "x");                        // insert(pos, const char*)
        expect(s, "xABC");

        s.insert(s.size(), "z");                 // pos == size_：尾部插入
        expect(s, "xABCz");

        s.insert(1, 2, '-');                     // insert(pos, count, ch)
        expect(s, "x--ABCz");

        String t("AD");
        String::iterator it = t.insert(t.begin() + 1, 'B');   // insert(const_iterator, char)
        CHECK(it == t.begin() + 1);
        CHECK(*it == 'B');
        expect(t, "ABD");

        String u("AD");
        String::iterator it2 = u.insert(u.end(), 2, '!');     // insert(const_iterator, count, ch)
        expect(u, "AD!!");
        CHECK(*it2 == '!');

        String emp;
        emp.insert(emp.begin(), 'q');            // 空对象上插入（begin() 是 nullptr）
        expect(emp, "q");

        String alias("XY");                      // 自插入
        alias.insert(0, alias);
        expect(alias, "XYXY");

        String alias2("XY");                     // 自别名 C 字符串插入
        alias2.insert(1, alias2.c_str());
        expect(alias2, "XXYY");

        // 与 std::string 对照。
        // 注意：iterator 就是 char*，字面量 0 既能当整数又是空指针常量，
        // 因此 insert(0, n, ch) 会在 (size_type, size_type, char) 和
        // (const_iterator, size_type, char) 之间产生歧义。
        // 这里用显式 size_type 变量规避（这是裸指针做迭代器的固有代价）。
        const String::size_type zero = 0;

        std::string o = "hello";
        String      h("hello");
        o.insert(2, "XYZ");   h.insert(2, "XYZ");
        CHECK(to_std(h) == o);
        o.insert(0, 3, '#');  h.insert(zero, 3, '#');
        CHECK(to_std(h) == o);

        expect_throws([&]{ String x("ab"); x.insert(9, "z"); });
        expect_throws([&]{ String x("ab"); x.insert(9, 1, 'z'); });
        expect_throws([&]{ String x("ab"); x.insert(9, String("z")); });
    }

    // ---------------- erase ----------------
    section("erase");

    {
        // 同理，字面量 0 会让 erase(0, 0) 在 (size_type, size_type) 和
        // (const_iterator, const_iterator) 之间产生歧义。
        const String::size_type zero = 0;

        String s("hello world");
        CHECK(&s.erase(5, 6) == &s);             // erase(pos, count)
        expect(s, "hello");

        s.erase(zero, zero);                     // count == 0：空操作
        expect(s, "hello");

        s.erase(s.size(), 5);                    // pos == size_：合法空操作
        expect(s, "hello");

        s.erase(3);                              // 只给 pos，默认 count = npos -> 截断
        expect(s, "hel");

        String t("abcdef");
        String::iterator it = t.erase(t.begin() + 1);        // erase(const_iterator)
        expect(t, "acdef");
        CHECK(*it == 'c');

        String u("abcdef");
        String::iterator it2 = u.erase(u.begin() + 1, u.begin() + 4);   // erase(first, last)
        expect(u, "aef");
        CHECK(*it2 == 'e');

        String v("abc");
        v.erase(v.begin(), v.begin());           // 空区间
        expect(v, "abc");

        String emp;
        emp.erase(emp.begin(), emp.end());       // 空串上的空区间（迭代器都是 nullptr）
        CHECK(emp.empty());

        String tail("abcdef");
        tail.erase(tail.end() - 2, tail.end());  // 删尾部
        expect(tail, "abcd");

        // 与 std::string 对照
        std::string o = "hello world";
        String      h("hello world");
        o.erase(2, 4);  h.erase(2, 4);
        CHECK(to_std(h) == o);
        o.erase(3);     h.erase(3);
        CHECK(to_std(h) == o);

        expect_throws([&]{ String x("ab"); x.erase(9, 1); });
        expect_throws([&]{ String x("ab"); x.erase(x.begin() + 2); });
        expect_throws([&]{ String x("ab"); x.erase(x.begin() + 2, x.begin() + 1); });
    }

    // ---------------- replace ----------------
    section("replace");

    {
        String s("hello world");
        CHECK(&s.replace(6, 5, String("there")) == &s);   // replace(pos, count, const Hstring&)
        expect(s, "hello there");

        s.replace(0, 5, "HELLO");                // replace(pos, count, const char*)
        expect(s, "HELLO there");

        s.replace(5, 6, 0, '#');                 // replace(pos, count, count2, ch)
        expect(s, "HELLO");

        s.replace(2, 2, "-");                    // 变短
        expect(s, "HE-O");

        s.replace(3, 1, "12345");                // 变长（触发扩容路径）
        expect(s, "HE-12345");                   // 注意：位置 3 的 'O' 被替换掉了

        s.replace(0, 0, ">>");                   // 纯插入
        expect(s, ">>HE-12345");

        s.replace(s.size(), 3, "<");             // pos == size_：纯追加
        expect(s, ">>HE-12345<");

        s.replace(2, 100, "");                   // 区间超长 -> 截断到末尾
        expect(s, ">>");

        String t("abcdef");                      // 自替换
        t.replace(2, 2, t);
        expect(t, "ababcdefef");

        String emp;
        emp.replace(0, 0, "start");              // 空对象上替换
        expect(emp, "start");

        // 与 std::string 对照
        std::string o = "abcdefghij";
        String      h("abcdefghij");
        o.replace(1, 3, "XY");        h.replace(1, 3, "XY");
        CHECK(to_std(h) == o);
        o.replace(0, 2, 5, '@');      h.replace(0, 2, 5, '@');
        CHECK(to_std(h) == o);
        o.replace(4, 100, "end");     h.replace(4, 100, "end");
        CHECK(to_std(h) == o);

        expect_throws([&]{ String x("ab"); x.replace(9, 1, "z"); });
        expect_throws([&]{ String x("ab"); x.replace(9, 1, 1, 'z'); });
    }

    // ---------------- swap ----------------
    section("swap");

    {
        String a("aaa"), b("bbb");
        a.swap(b);                               // 成员 swap
        expect(a, "bbb");
        expect(b, "aaa");

        String c("ccc"), d("ddd");
        swap(c, d);                              // 非成员 swap（ADL）
        expect(c, "ddd");
        expect(d, "ccc");

        String e("eee"), f;
        swap(e, f);                              // 与空对象交换
        expect(e, "");
        expect(f, "eee");

        String g("ggg");
        g.swap(g);                               // 自交换
        expect(g, "ggg");

        String h("short"), i("a much longer string to force reallocation");
        h.swap(i);                               // 交换后仍能正常析构
        expect(h, "a much longer string to force reallocation");
        expect(i, "short");
    }

    // ---------------- substr / copy ----------------
    section("substr and copy");

    {
        std::string o = "hello world";
        String      h("hello world");
        for(std::size_t pos : {std::size_t(0), std::size_t(5), std::size_t(11)}) {
            for(std::size_t cnt : {std::size_t(0), std::size_t(3), std::size_t(100)}) {
                CHECK(to_std(h.substr(pos, cnt)) == o.substr(pos, cnt));
            }
        }
        CHECK(to_std(h.substr()) == o.substr());      // 默认 pos = 0, count = npos
        CHECK(to_std(h.substr(6)) == o.substr(6));
        CHECK(to_std(String().substr(0)) == "");      // 空对象上取子串
        expect_throws([&]{ String("ab").substr(9); });

        char buf[64];
        std::string oa = "hello world";
        String      ha("hello world");
        for(std::size_t pos : {std::size_t(0), std::size_t(3), std::size_t(11)}) {
            for(std::size_t cnt : {std::size_t(0), std::size_t(4), std::size_t(100)}) {
                CHECK(ha.copy(buf, cnt, pos) == oa.copy(buf, cnt, pos));
            }
        }

        String hb("hello world");
        String::size_type n = hb.copy(buf, 5, 6);
        CHECK(n == 5);
        CHECK(std::string(buf, n) == "world");

        CHECK(String("abc").copy(buf, 0, 0) == 0);     // count == 0
        CHECK(String("abc").copy(buf, 5, 3) == 0);     // pos == size_ -> 长度 0
        expect_throws([&]{ String("ab").copy(buf, 1, 9); });
    }

    // ---------------- compare ----------------
    section("compare");

    {
        const char* samples[] = {"", "a", "ab", "abc", "abd", "b", "abcd"};
        for(const char* x : samples) {
            for(const char* y : samples) {
                std::string ox(x), oy(y);
                String      hx(x), hy(y);
                // 只比较符号：正负与 std::string 一致即可（不要求恰好 -1/0/1）
                CHECK((hx.compare(hy) < 0) == (ox.compare(oy) < 0));
                CHECK((hx.compare(hy) > 0) == (ox.compare(oy) > 0));
                CHECK((hx.compare(hy) == 0) == (ox.compare(oy) == 0));
                CHECK((hx.compare(y) < 0) == (ox.compare(oy) < 0));
                CHECK((hx.compare(y) > 0) == (ox.compare(oy) > 0));
                CHECK((hx.compare(y) == 0) == (ox.compare(oy) == 0));
            }
        }

        std::string ob = "abcdef";
        String      hb("abcdef");
        for(std::size_t pos : {std::size_t(0), std::size_t(2), std::size_t(6)}) {
            for(std::size_t cnt : {std::size_t(0), std::size_t(1),
                                   std::size_t(3), std::size_t(100)}) {
                std::string oa = "abcxyz";
                String      ha("abcxyz");
                CHECK((ha.compare(pos, cnt, hb) < 0) == (oa.compare(pos, cnt, ob) < 0));
                CHECK((ha.compare(pos, cnt, hb) > 0) == (oa.compare(pos, cnt, ob) > 0));
                CHECK((ha.compare(pos, cnt, hb) == 0) == (oa.compare(pos, cnt, ob) == 0));
            }
        }
        expect_throws([&]{ String("ab").compare(9, 1, String("x")); });
    }

    // ---------------- find / rfind / find_*_of ----------------
    section("find / rfind / find_*_of");

    {
        const char* texts[]   = {"", "a", "abcabc", "hello world", "aaaa"};
        const char* needles[] = {"", "a", "abc", "xyz", "z", "aaaa", "abcd"};
        const char  chars[]   = {'a', 'z', '\0'};

        for(const char* t : texts) {
            for(const char* n : needles) {
                std::string o(t);
                String      h(t);
                for(std::size_t pos : {std::size_t(0), std::size_t(1),
                                       std::size_t(3), std::size_t(100)}) {
                    CHECK(h.find(String(n), pos)     == o.find(n, pos));
                    CHECK(h.find(n, pos)             == o.find(n, pos));
                    CHECK(h.rfind(String(n), pos)    == o.rfind(n, pos));
                    CHECK(h.rfind(n, pos)            == o.rfind(n, pos));
                    CHECK(h.find_first_of(String(n), pos)     == o.find_first_of(n, pos));
                    CHECK(h.find_first_of(n, pos)             == o.find_first_of(n, pos));
                    CHECK(h.find_last_of(String(n), pos)      == o.find_last_of(n, pos));
                    CHECK(h.find_last_of(n, pos)              == o.find_last_of(n, pos));
                    CHECK(h.find_first_not_of(String(n), pos) == o.find_first_not_of(n, pos));
                    CHECK(h.find_first_not_of(n, pos)         == o.find_first_not_of(n, pos));
                    CHECK(h.find_last_not_of(String(n), pos)  == o.find_last_not_of(n, pos));
                    CHECK(h.find_last_not_of(n, pos)          == o.find_last_not_of(n, pos));
                }
                // 默认实参版本：pos 分别为 0 和 npos
                CHECK(h.find(String(n))              == o.find(n));
                CHECK(h.rfind(String(n))             == o.rfind(n));
                CHECK(h.find_first_of(String(n))     == o.find_first_of(n));
                CHECK(h.find_last_of(String(n))      == o.find_last_of(n));
                CHECK(h.find_first_not_of(String(n)) == o.find_first_not_of(n));
                CHECK(h.find_last_not_of(String(n))  == o.find_last_not_of(n));
            }
            for(char ch : chars) {
                std::string o(t);
                String      h(t);
                for(std::size_t pos : {std::size_t(0), std::size_t(2), std::size_t(100)}) {
                    CHECK(h.find(ch, pos)               == o.find(ch, pos));
                    CHECK(h.rfind(ch, pos)              == o.rfind(ch, pos));
                    CHECK(h.find_first_of(ch, pos)      == o.find_first_of(ch, pos));
                    CHECK(h.find_last_of(ch, pos)       == o.find_last_of(ch, pos));
                    CHECK(h.find_first_not_of(ch, pos)  == o.find_first_not_of(ch, pos));
                    CHECK(h.find_last_not_of(ch, pos)   == o.find_last_not_of(ch, pos));
                }
                CHECK(h.find(ch)  == o.find(ch));
                CHECK(h.rfind(ch) == o.rfind(ch));
            }
        }

        CHECK(String::npos == std::string::npos);
        CHECK(String("abc").find("z") == String::npos);
        CHECK(String("abc").find("", 5) == String::npos);   // pos 超长时空子串也找不到
    }

    // ---------------- 比较运算符 ----------------
    section("comparison operators");

    {
        const char* samples[] = {"", "a", "ab", "abc", "abd", "b", "abcd"};
        for(const char* x : samples) {
            for(const char* y : samples) {
                std::string ox(x), oy(y);
                String      hx(x), hy(y);

                CHECK((hx == hy) == (ox == oy));
                CHECK((hx != hy) == (ox != oy));
                CHECK((hx <  hy) == (ox <  oy));
                CHECK((hx <= hy) == (ox <= oy));
                CHECK((hx >  hy) == (ox >  oy));
                CHECK((hx >= hy) == (ox >= oy));

                CHECK((hx == y) == (ox == oy));      // Hstring 与 const char*
                CHECK((x == hy) == (ox == oy));      // const char* 与 Hstring
                CHECK((hx != y) == (ox != oy));
                CHECK((x != hy) == (ox != oy));
            }
        }
        // 前缀关系是原实现最容易出错的地方："abc" < "abcd" 必须为真
        CHECK(String("abc") < String("abcd"));
        CHECK(!(String("abcd") < String("abc")));
        // 关系运算符只有 (Hstring, Hstring) 重载，混用 C 字符串需要显式构造
        CHECK(String("abc") < String("abcd"));
    }

    // ---------------- operator+ ----------------
    section("operator+");

    {
        String a("ab"), b("cd");
        expect(a + b,   "abcd");                 // (Hstring, Hstring)
        expect(a + "cd", "abcd");                // (Hstring, const char*)
        expect("ab" + b, "abcd");                // (const char*, Hstring)
        expect(a + '!', "ab!");                  // (Hstring, char)
        expect('!' + a, "!ab");                  // (char, Hstring)
        expect(String() + String(), "");         // 两个空对象
        expect(a + "", "ab");                    // 空 C 字符串
        expect("" + a, "ab");
        CHECK(a.size() == 2);                    // 原对象不被修改
        CHECK(b.size() == 2);
    }

    // ---------------- 流输入输出 ----------------
    section("stream input / output");

    {
        std::ostringstream os;
        os << String("hello") << '-' << String() << '-' << String(2, 'x');
        CHECK(os.str() == "hello--xx");

        String nul_str;                          // 内容含内嵌 '\0'
        nul_str.assign("nul\0inside", 9);
        CHECK(nul_str.size() == 9);
        std::ostringstream os2;
        os2 << nul_str;
        CHECK(os2.str().size() == 9);            // 必须按长度写出，不能写到遇到 '\0' 就停
        CHECK(os2.str() == std::string("nul\0insid", 9));

        std::istringstream is("  hello   world  ");
        String a, b, c;
        is >> a >> b;                            // operator>> 跳过前导空白
        CHECK(to_std(a) == "hello");
        CHECK(to_std(b) == "world");
        is >> c;                                 // 已到末尾
        CHECK(is.fail());
        CHECK(c.empty());

        std::istringstream dummy("x");
        String big(100, 'b');
        dummy >> big;                            // operator>> 应先清空目标
        CHECK(to_std(big) == "x");

        std::istringstream gl("first line\nsecond\n\nfourth");
        String l1, l2, l3, l4;
        CHECK(static_cast<bool>(getline(gl, l1)));   // getline(is, str)
        getline(gl, l2);
        getline(gl, l3);
        getline(gl, l4);
        CHECK(to_std(l1) == "first line");
        CHECK(to_std(l2) == "second");
        CHECK(to_std(l3) == "");                     // 空行
        CHECK(to_std(l4) == "fourth");

        std::istringstream gl2("a,b,c");
        String p1, p2;
        getline(gl2, p1, ',');                       // getline(is, str, delim)
        getline(gl2, p2, ',');
        CHECK(to_std(p1) == "a");
        CHECK(to_std(p2) == "b");

        String whole;
        std::istringstream gl3("no delimiter here");
        getline(gl3, whole);
        CHECK(to_std(whole) == "no delimiter here");

        String kept("stale");
        std::istringstream gl4("fresh");
        getline(gl4, kept);                          // 应先清空目标
        CHECK(to_std(kept) == "fresh");
    }

    std::cout << (g_failures == 0 ? "All Hstring tests passed.\n"
                                  : "SOME TESTS FAILED\n");
    return g_failures == 0 ? 0 : 1;
}
