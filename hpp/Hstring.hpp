#ifndef HSTRING_HPP
#define HSTRING_HPP

#include <cstddef>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <algorithm>
#include <limits>

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

private:
    char* data_;
    size_type size_;
    size_type capacity_;
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

#endif // HSTRING_HPP