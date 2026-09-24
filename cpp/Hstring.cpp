#include "hpp/Hstring.hpp"
#include <algorithm>
#include <string>
#include <utility>

template<typename Allocator>
Hstring<Allocator>::Hstring():data_(nullptr), size_(0), capacity_(0) {}

template<typename Allocator>
Hstring<Allocator>::Hstring(const char* s):data_(nullptr), size_(0), capacity_(0) {
    if(!s)  return ;
    const size_type n = std::char_traits<char>::length(s);
    Allocator alloc;
    data_ = alloc.allocate(n + 1);
    std::char_traits<char>::copy(data_, s, n);
    data_[n] = '\0';
    size_ = n;
    capacity_ = n + 1;
}

template<typename Allocator>
Hstring<Allocator>::Hstring(const char* s, size_type count):data_(nullptr), size_(0), capacity_(0) {
    if(!s)  return ;
    const size_type n = std::char_traits<char>::length(s);
    const size_type len = std::min(count, n);
    Allocator alloc;
    data_ = alloc.allocate(len + 1);
    std::char_traits<char>::copy(data_, s, len);
    data_[len] = '\0';
    size_ = len;
    capacity_ = len + 1;
}

template<typename Allocator>
Hstring<Allocator>::Hstring(size_type count, char ch):data_(nullptr), size_(0), capacity_(0) {
    Allocator alloc;
    data_ = alloc.allocate(count + 1);
    for(size_type i = 0; i < count; ++i) {
        data_[i] = ch;
    }
    data_[count] = '\0';
    size_ = count;
    capacity_ = count + 1;
}

template<typename Allocator>
Hstring<Allocator>::Hstring(const Hstring& other):data_(nullptr), size_(0), capacity_(0) {
    if(other.data_ == nullptr)  return ;
    Allocator alloc;
    data_ = alloc.allocate(other.capacity_);
    std::char_traits<char>::copy(data_, other.data_, other.capacity_);
    size_ = other.size_;
    capacity_ = other.capacity_;
}

template<typename Allocator>
Hstring<Allocator>::Hstring(Hstring&& other) noexcept
    :data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

template<typename Allocator>
Hstring<Allocator>::Hstring(std::initializer_list<char> ilist):data_(nullptr), size_(0), capacity_(0) {
    const size_type n = ilist.size();
    Allocator alloc;
    data_ = alloc.allocate(n + 1);
    size_type i = 0;
    for(auto it = ilist.begin(); it != ilist.end(); ++it, ++i) {
        data_[i] = *it;
    }
    data_[n] = '\0';
    size_ = n;
    capacity_ = n + 1;
}

template<typename Allocator>
Hstring<Allocator>::~Hstring() {
    if(data_ != nullptr) {
        Allocator alloc;
        alloc.deallocate(data_, capacity_);
    }
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(const Hstring& other) {
    if(this == &other)  return *this;
    Hstring tmp(other);
    std::swap(data_, tmp.data_);
    std::swap(size_, tmp.size_);
    std::swap(capacity_, tmp.capacity_);
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(Hstring&& other) noexcept {
    if(this == &other)  return *this;
    if(data_ != nullptr) {
        Allocator alloc;
        alloc.deallocate(data_, capacity_);
    }
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(const char* s) {
    size_type n = std::char_traits<char>::length(s);
    if(size_ < n) {
        Allocator alloc;
        alloc.deallocate(data_, capacity_);
        data_ = alloc.allocate(n + 1);
        capacity_ = n + 1;
    }
    std::char_traits<char>::copy(data_, s, n);
    data_[n] = '\0';
    size_ = n;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(char ch) {
    if(!data_) {
        Allocator alloc;
        data_ = alloc.allocate(2);
        capacity_ = 2;
    }
    data_[0] = ch; data_[1] = '\0';
    size_ = 1;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(std::initializer_list<char> ilist) {
    size_type n = ilist.size();
    if(size_ < n) {
        Allocator alloc;
        alloc.deallocate(data_, capacity_);
        data_ = alloc.allocate(n + 1);
        capacity_ = n + 1;
    }
    size_type i = 0;
    for(auto it = ilist.begin(); it != ilist.end(); ++it, ++i) {
        data_[i] = *it;
    }
    data_[n] = '\0';
    size_ = n;
    return *this;
}


// 显式实例化：把定义生成到这个 TU 里，供其他翻译单元链接使用。
// 注意：泛型迭代器构造函数定义在头文件中，不需要（也无法）在此实例化。
template Hstring<std::allocator<char>>::Hstring();
template Hstring<std::allocator<char>>::Hstring(const char*);
template Hstring<std::allocator<char>>::Hstring(const char*, size_type);
template Hstring<std::allocator<char>>::Hstring(size_type, char);
template Hstring<std::allocator<char>>::Hstring(const Hstring&);
template Hstring<std::allocator<char>>::Hstring(Hstring&&) noexcept;
template Hstring<std::allocator<char>>::Hstring(std::initializer_list<char>);
template Hstring<std::allocator<char>>::~Hstring();
template Hstring<std::allocator<char>>& Hstring<std::allocator<char>>::operator=(const Hstring&);
template Hstring<std::allocator<char>>& Hstring<std::allocator<char>>::operator=(Hstring&&) noexcept;
template Hstring<std::allocator<char>>& Hstring<std::allocator<char>>::operator=(const char* s);
template Hstring<std::allocator<char>>& Hstring<std::allocator<char>>::operator=(char ch);
template Hstring<std::allocator<char>>& Hstring<std::allocator<char>>::operator=(std::initializer_list<char> ilist);