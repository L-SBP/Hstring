#include "hpp/Hstring.hpp"
#include <algorithm>
#include <string>
#include <utility>
#include <stdexcept>
#include <functional>

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
    if(s == nullptr)  return *this;
    // 处理自别名（例如 s = s.c_str()）：s 可能指向本对象自己的缓冲区，
    // 若先 reserve 重新分配，s 就会变成悬空指针。
    if(data_ != nullptr
       && !std::less<const char*>()(s, data_)
       && std::less<const char*>()(s, data_ + size_)) {
        const std::string tmp(s);
        return *this = tmp.c_str();
    }
    const size_type n = std::char_traits<char>::length(s);
    if(n + 1 > capacity_) {
        reserve(n + 1);
    }
    std::char_traits<char>::copy(data_, s, n);
    data_[n] = '\0';
    size_ = n;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(char ch) {
    if(capacity_ < 2)  reserve(2);
    data_[0] = ch; data_[1] = '\0';
    size_ = 1;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator=(std::initializer_list<char> ilist) {
    const size_type n = ilist.size();
    if(n + 1 > capacity_) {
        reserve(n + 1);
    }
    size_type i = 0;
    for(auto it = ilist.begin(); it != ilist.end(); ++it, ++i) {
        data_[i] = *it;
    }
    data_[n] = '\0';
    size_ = n;
    return *this;
}

template<typename Allocator>
void Hstring<Allocator>::reserve(size_type new_cap) {
    if(capacity_ >= new_cap)  return;
    Allocator alloc;
    char* new_data = alloc.allocate(new_cap);
    if(data_ != nullptr) {
        std::char_traits<char>::copy(new_data, data_, size_ + 1);
        alloc.deallocate(data_, capacity_);
    } else {
        new_data[0] = '\0';
    }
    data_ = new_data;
    capacity_ = new_cap;
}

template<typename Allocator>
void Hstring<Allocator>::shrink_to_fit() {
    if(data_ == nullptr)  return;
    if(capacity_ == size_ + 1)  return;
    Allocator alloc;
    char* new_data = alloc.allocate(size_ + 1);
    std::char_traits<char>::copy(new_data, data_, size_ + 1);
    alloc.deallocate(data_, capacity_);
    data_ = new_data;
    capacity_ = size_ + 1;
}

template<typename Allocator>
void Hstring<Allocator>::resize(size_type count) {
    resize(count, '\0');
}

template<typename Allocator>
void Hstring<Allocator>::resize(size_type count, char ch) {
    // 预留 count + 1 个槽位：count 个字符加上结尾的 '\0'。
    if(count + 1 > capacity_) {
        reserve(count + 1);
    }
    if(count > size_) {
        for(size_type i = size_; i < count; ++i) {
            data_[i] = ch;
        }
    }
    size_ = count;
    data_[size_] = '\0';
}

template<typename Allocator>
void Hstring<Allocator>::clear() noexcept {
    if(data_ != nullptr) {
        Allocator alloc;
        alloc.deallocate(data_, capacity_);
    }
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
}

template<typename Allocator>
char& Hstring<Allocator>::operator[](size_type idx) {
    return data_[idx];
}

template<typename Allocator>
const char& Hstring<Allocator>::operator[](size_type idx) const {
    return data_[idx];
}

template<typename Allocator>
char& Hstring<Allocator>::at(size_type idx) {
    if(idx >= size_)    throw std::out_of_range("The index is out of range");
    return data_[idx];
}

template<typename Allocator>
const char& Hstring<Allocator>::at(size_type idx) const {
    if(idx >= size_)    throw std::out_of_range("The index is out of range");
    return data_[idx];
}

template<typename Allocator>
char& Hstring<Allocator>::front() {
    if(size_ == 0)  throw std::out_of_range("The contaner is empty");
    return data_[0];
}

template<typename Allocator>
const char& Hstring<Allocator>::front() const {
    if(size_ == 0)  throw std::out_of_range("The contaner is empty");
    return data_[0];
}

template<typename Allocator>
char& Hstring<Allocator>::back() {
    if(size_ == 0)  throw std::out_of_range("The contaner is empty");
    return data_[size_ - 1];
}

template<typename Allocator>
const char& Hstring<Allocator>::back() const {
    if(size_ == 0)  throw std::out_of_range("The contaner is empty");
    return data_[size_ - 1];
}

template<typename Allocator>
char* Hstring<Allocator>::data() noexcept {
    return data_;
}

template<typename Allocator>
const char* Hstring<Allocator>::data() const noexcept {
    return data_;
}

template<typename Allocator>
const char* Hstring<Allocator>::c_str() const noexcept {
    return data_ != nullptr ? data_ : "";
}

template<typename Allocator>
typename Hstring<Allocator>::iterator Hstring<Allocator>::begin() noexcept {
    return data_;
}

template<typename Allocator>
typename Hstring<Allocator>::const_iterator Hstring<Allocator>::begin() const noexcept {
    return data_;
}

template<typename Allocator>
typename Hstring<Allocator>::const_iterator Hstring<Allocator>::cbegin() const noexcept {
    return data_;
}

template<typename Allocator>
typename Hstring<Allocator>::iterator Hstring<Allocator>::end() noexcept {
    return data_ + size_;
}

template<typename Allocator>
typename Hstring<Allocator>::const_iterator Hstring<Allocator>::end() const noexcept {
    return data_ + size_;
}

template<typename Allocator>
typename Hstring<Allocator>::const_iterator Hstring<Allocator>::cend() const noexcept {
    return data_ + size_;
}

template<typename Allocator>
typename Hstring<Allocator>::reverse_iterator Hstring<Allocator>::rbegin() noexcept {
    return reverse_iterator(end());
}

template<typename Allocator>
typename Hstring<Allocator>::const_reverse_iterator Hstring<Allocator>::rbegin() const noexcept {
    return const_reverse_iterator(end());
}

template<typename Allocator>
typename Hstring<Allocator>::const_reverse_iterator Hstring<Allocator>::crbegin() const noexcept {
    return const_reverse_iterator(cend());
}

template<typename Allocator>
typename Hstring<Allocator>::reverse_iterator Hstring<Allocator>::rend() noexcept {
    return reverse_iterator(begin());
}

template<typename Allocator>
typename Hstring<Allocator>::const_reverse_iterator Hstring<Allocator>::rend() const noexcept {
    return const_reverse_iterator(begin());
}

template<typename Allocator>
typename Hstring<Allocator>::const_reverse_iterator Hstring<Allocator>::crend() const noexcept {
    return const_reverse_iterator(cbegin());
}

template<typename Allocator>
Hstring& operator+=(const Hstring& str) {
    size_t new_sz = size_ + str.length();
    if(new_sz + 1 <= capacity_) {
        std::char_traits<char>::copy(data_ + size_, str.data(), str.length());
    } else {
        Allocator alloc;
        char* new_data = alloc.allocate(new_sz + 1);
        capacity_ = new_sz + 1;
        std::char_traits<char>::copy(new_data, data_, size_);
        std::char_traits<char>::copy(new_data + size_, str.data(), str.length());
    }
    data_[new_sz] = '\0';
    size_ = new_sz;
}

template<typename Allocator>
Hstring& operator+=(const char* s) {

}

template<typename Allocator>
Hstring& operator+=(char ch) {

}

template<typename Allocator>
Hstring& operator+=(std::initializer_list<char> ilist) {

}


// 所有非模板成员都已有定义，因此可以整体实例化。
// 泛型迭代器构造函数是成员模板，不参与类级实例化，其定义在头文件中。
template class Hstring<std::allocator<char>>;
