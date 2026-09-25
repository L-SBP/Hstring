#ifndef HSTRING_HPP
#define HSTRING_HPP

#include <cstddef>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <algorithm>
#include <limits>
#include <string>     // std::char_traits
#include <ostream>    // operator<<
#include <istream>    // operator>>, getline
#include <cctype>     // std::isspace

template<typename Allocator = std::allocator<char>>
class Hstring {
public:
    using value_type      = char;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = char&;
    using const_reference = const char&;
    using pointer         = char*;
    using const_pointer   = const char*;
    using iterator        = char*;
    using const_iterator  = const char*;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type npos = size_type(-1);
    
    Hstring();
    Hstring(const char* s);
    Hstring(const char* s, size_type count);
    Hstring(size_type count, char ch);
    Hstring(const Hstring& other);
    Hstring(Hstring&& other) noexcept;
    template<typename InputIt>
    Hstring(InputIt first, InputIt last);
    Hstring(std::initializer_list<char> ilist);

    ~Hstring();

    Hstring& operator=(const Hstring& other);
    Hstring& operator=(Hstring&& other) noexcept;
    Hstring& operator=(const char* s);
    Hstring& operator=(char ch);
    Hstring& operator=(std::initializer_list<char> ilist);

    size_type size() const noexcept { return size_; }
    size_type length() const noexcept { return size_; }
    size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(char) - 1; }
    size_type capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    void reserve(size_type new_cap);
    void shrink_to_fit();
    void resize(size_type count);
    void resize(size_type count, char ch);
    void clear() noexcept;

    char& operator[](size_type idx);
    const char& operator[](size_type idx) const;

    char& at(size_type idx);
    const char& at(size_type idx) const;

    char& front();
    const char& front() const;

    char& back();
    const char& back() const;

    char* data() noexcept;
    const char* data() const noexcept;

    const char* c_str() const noexcept;

    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;

    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

    Hstring& operator+=(const Hstring& str);
    Hstring& operator+=(const char* s);
    Hstring& operator+=(char ch);
    Hstring& operator+=(std::initializer_list<char> ilist);

    Hstring& append(const Hstring& str);
    Hstring& append(const Hstring& str, size_type pos, size_type count = npos);
    Hstring& append(const char* s, size_type count);
    Hstring& append(const char* s);
    Hstring& append(size_type count, char ch);
    template<class InputIt>
    Hstring& append(InputIt first, InputIt last);

    void push_back(char ch);
    void pop_back();

    Hstring& assign(const Hstring& str);
    Hstring& assign(const char* s);
    Hstring& assign(const char* s, size_type count);
    Hstring& assign(size_type count, char ch);
    template<class InputIt>
    Hstring& assign(InputIt first, InputIt last);

    Hstring& insert(size_type pos, const Hstring& str);
    Hstring& insert(size_type pos, const char* s);
    Hstring& insert(size_type pos, size_type count, char ch);
    iterator insert(const_iterator pos, char ch);
    iterator insert(const_iterator pos, size_type count, char ch);

    Hstring& erase(size_type pos = 0, size_type count = npos);
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    Hstring& replace(size_type pos, size_type count, const Hstring& str);
    Hstring& replace(size_type pos, size_type count, const char* s);
    Hstring& replace(size_type pos, size_type count, size_type count2, char ch);

    void swap(Hstring& other) noexcept;

    Hstring substr(size_type pos = 0, size_type count = npos) const;

    size_type copy(char* dest, size_type count, size_type pos = 0) const;

    int compare(const Hstring& str) const;
    int compare(const char* s) const;
    int compare(size_type pos, size_type count, const Hstring& str) const;

    size_type find(const Hstring& str, size_type pos = 0) const;
    size_type find(const char* s, size_type pos = 0) const;
    size_type find(char ch, size_type pos = 0) const;

    size_type rfind(const Hstring& str, size_type pos = npos) const;
    size_type rfind(const char* s, size_type pos = npos) const;
    size_type rfind(char ch, size_type pos = npos) const;

    size_type find_first_of(const Hstring& str, size_type pos = 0) const;
    size_type find_first_of(const char* s, size_type pos = 0) const;
    size_type find_first_of(char ch, size_type pos = 0) const;

    size_type find_last_of(const Hstring& str, size_type pos = npos) const;
    size_type find_last_of(const char* s, size_type pos = npos) const;
    size_type find_last_of(char ch, size_type pos = npos) const;

    size_type find_first_not_of(const Hstring& str, size_type pos = 0) const;
    size_type find_first_not_of(const char* s, size_type pos = 0) const;
    size_type find_first_not_of(char ch, size_type pos = 0) const;

    size_type find_last_not_of(const Hstring& str, size_type pos = npos) const;
    size_type find_last_not_of(const char* s, size_type pos = npos) const;
    size_type find_last_not_of(char ch, size_type pos = npos) const;

    // 注意：比较运算符、operator+、operator<<、operator>>、getline 都声明为
    // 非成员函数（见类定义之后）。原因是它们的左操作数可能是 const char*，
    // 而成员函数的左操作数必须是本类对象，写成成员会编译失败。
private:
    char* data_;
    size_type size_;
    size_type capacity_;

    // 空串用 data_ == nullptr 表示，此时 begin() == end() == nullptr。
    // 对 nullptr 做指针相减是 UB，所以这里显式处理。
    size_type index_of(const_iterator it) const noexcept {
        return (data_ == nullptr) ? 0 : static_cast<size_type>(it - data_);
    }

    // 在 pos 处腾出 count 个字符的空位，把原内容（含结尾 '\0'）整体后移。
    // src 非空时顺便拷入 src 的 count 个字符；src 为 nullptr 时只腾空间。
    Hstring& insert_raw(size_type pos, const char* src, size_type count);

    // 用 src 的 n 个字符替换 [pos, pos + len) 区间。
    // 调用者需保证 pos <= size_ 且 len <= size_ - pos。
    Hstring& replace_raw(size_type pos, size_type len, const char* src, size_type n);
};

template<typename Allocator>
template<typename InputIt>
Hstring<Allocator>::Hstring(InputIt first, InputIt last)
    : data_(nullptr), size_(0), capacity_(0) {
    // 单遍遍历 + 容量倍增：不能用 std::distance 先量长度，
    // 那会消耗掉 istream_iterator 这类单遍输入迭代器。
    Allocator alloc;
    char* buf = nullptr;
    size_type cap = 0;
    size_type n = 0;
    for(InputIt curr = first; curr != last; ++curr) {
        if(n + 1 >= cap) {
            const size_type new_cap = (cap == 0) ? size_type(8) : cap * 2;
            char* new_buf = alloc.allocate(new_cap);
            if(buf != nullptr) {
                std::char_traits<char>::copy(new_buf, buf, n);
                alloc.deallocate(buf, cap);
            }
            buf = new_buf;
            cap = new_cap;
        }
        buf[n++] = *curr;
    }
    if(buf == nullptr) {
        buf = alloc.allocate(1);
        cap = 1;
    }
    buf[n] = '\0';
    data_ = buf;
    size_ = n;
    capacity_ = cap;
}

template<typename Allocator>
template<class InputIt>
Hstring<Allocator>& Hstring<Allocator>::append(InputIt first, InputIt last) {
    // 单遍遍历：不能用 std::distance 预先量长度，那会消耗掉
    // istreambuf_iterator 这类单遍输入迭代器。这里交给 push_back 处理扩容。
    for(InputIt curr = first; curr != last; ++curr) {
        push_back(static_cast<char>(*curr));
    }
    return *this;
}

template<typename Allocator>
template<class InputIt>
Hstring<Allocator>& Hstring<Allocator>::assign(InputIt first, InputIt last) {
    clear();
    for(InputIt curr = first; curr != last; ++curr) {
        push_back(static_cast<char>(*curr));
    }
    return *this;
}

// ---------------- 非成员比较运算符 ----------------
// 这些运算符只使用公有接口（size / compare），因此不需要声明为 friend。
// 定义在头文件中是为了能对任意 Allocator 实例化。

template<typename Allocator>
inline bool operator==(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    return lhs.size() == rhs.size() && lhs.compare(rhs) == 0;
}

template<typename Allocator>
inline bool operator==(const Hstring<Allocator>& lhs, const char* rhs) {
    return lhs.compare(rhs) == 0;
}

template<typename Allocator>
inline bool operator==(const char* lhs, const Hstring<Allocator>& rhs) {
    return rhs.compare(lhs) == 0;
}

template<typename Allocator>
inline bool operator!=(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    return !(lhs == rhs);
}

template<typename Allocator>
inline bool operator!=(const Hstring<Allocator>& lhs, const char* rhs) {
    return !(lhs == rhs);
}

template<typename Allocator>
inline bool operator!=(const char* lhs, const Hstring<Allocator>& rhs) {
    return !(lhs == rhs);
}

// 注意：这些关系运算符必须复用 compare()，不能只比较前 min(len) 个字符——
// 那样会把 "abc" 和 "abcd" 判为相等，于是 "abc" < "abcd" 得到 false（错误）。
template<typename Allocator>
inline bool operator<(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    return lhs.compare(rhs) < 0;
}

template<typename Allocator>
inline bool operator<=(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    return lhs.compare(rhs) <= 0;
}

template<typename Allocator>
inline bool operator>(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    return lhs.compare(rhs) > 0;
}

template<typename Allocator>
inline bool operator>=(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    return lhs.compare(rhs) >= 0;
}

// ---------------- 非成员拼接运算符 ----------------

template<typename Allocator>
inline Hstring<Allocator> operator+(const Hstring<Allocator>& lhs, const Hstring<Allocator>& rhs) {
    Hstring<Allocator> result;
    result.reserve(lhs.size() + rhs.size() + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
}

template<typename Allocator>
inline Hstring<Allocator> operator+(const Hstring<Allocator>& lhs, const char* rhs) {
    Hstring<Allocator> result;
    result.reserve(lhs.size() + std::char_traits<char>::length(rhs) + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
}

template<typename Allocator>
inline Hstring<Allocator> operator+(const char* lhs, const Hstring<Allocator>& rhs) {
    Hstring<Allocator> result;
    result.reserve(std::char_traits<char>::length(lhs) + rhs.size() + 1);
    result.append(lhs);
    result.append(rhs);
    return result;
}

template<typename Allocator>
inline Hstring<Allocator> operator+(const Hstring<Allocator>& lhs, char rhs) {
    Hstring<Allocator> result;
    result.reserve(lhs.size() + 2);
    result.append(lhs);
    result.push_back(rhs);
    return result;
}

template<typename Allocator>
inline Hstring<Allocator> operator+(char lhs, const Hstring<Allocator>& rhs) {
    Hstring<Allocator> result;
    result.reserve(rhs.size() + 2);
    result.push_back(lhs);
    result.append(rhs);
    return result;
}

template<typename Allocator>
inline void swap(Hstring<Allocator>& lhs, Hstring<Allocator>& rhs) noexcept {
    lhs.swap(rhs);
}

// ---------------- 流输入输出 ----------------

template<typename Allocator>
inline std::ostream& operator<<(std::ostream& os, const Hstring<Allocator>& str) {
    if(str.size() != 0) {
        os.write(str.data(), static_cast<std::streamsize>(str.size()));
    }
    return os;
}

template<typename Allocator>
inline std::istream& operator>>(std::istream& is, Hstring<Allocator>& str) {
    // 与 std::string 一致：跳过前导空白，读到下一个空白字符为止。
    str.clear();
    std::istream::sentry sen(is);
    if(!sen)  return is;
    char c;
    while(is.get(c)) {
        if(std::isspace(static_cast<unsigned char>(c))) {
            is.unget();
            break;
        }
        str.push_back(c);
    }
    return is;
}

template<typename Allocator>
inline std::istream& getline(std::istream& is, Hstring<Allocator>& str, char delim) {
    str.clear();
    std::istream::sentry sen(is, true);   // true：不跳过前导空白
    if(!sen)  return is;
    char c;
    while(is.get(c)) {
        if(c == delim)  break;
        str.push_back(c);
    }
    return is;
}

template<typename Allocator>
inline std::istream& getline(std::istream& is, Hstring<Allocator>& str) {
    return getline(is, str, '\n');
}

#endif // HSTRING_HPP