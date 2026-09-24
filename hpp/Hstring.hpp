#ifndef HSTRING_HPP
#define HSTRING_HPP

#include <cstddef>
#include <memory>
#include <iterator>
#include <initializer_list>
#include <algorithm>

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
    size_type capacity() const noexcept { return capacity_; }
    const_pointer data() const noexcept { return data_; }
    const_pointer c_str() const noexcept { return data_ ? data_ : ""; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ ? data_ + size_ : data_; }

private:
    char* data_;
    size_type size_;
    size_type capacity_;
};

template<typename Allocator>
template<typename InputIt>
Hstring<Allocator>::Hstring(InputIt first, InputIt last)
    : data_(nullptr), size_(0), capacity_(0) {
    const size_type n = static_cast<size_type>(std::distance(first, last));
    Allocator alloc;
    data_ = alloc.allocate(n + 1);
    size_type i = 0;
    for(InputIt curr = first; curr != last; ++curr, ++i) {
        data_[i] = *curr;
    }
    data_[n] = '\0';
    size_ = n;
    capacity_ = n + 1;
}

#endif // HSTRING_HPP