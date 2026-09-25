#include "hpp/Hstring.hpp"
#include <algorithm>
#include <string>
#include <utility>
#include <stdexcept>
#include <functional>

namespace {
// 判断 p 是否落在 [begin, begin + len) 范围内，用于识别自别名：
// 源数据就是本对象自己的缓冲区，一旦扩容源指针就会失效。
// 用 std::less 比较指针，避免未定义行为的裸比较。
inline bool points_into(const char* p, const char* begin, std::size_t len) noexcept {
    if(begin == nullptr || p == nullptr)  return false;
    return !std::less<const char*>()(p, begin)
        && std::less<const char*>()(p, begin + len);
}
}  // namespace

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

// 这 4 个运算符统一委托给 append / push_back，避免重复实现扩容逻辑。
// 原实现有三个问题：扩容后没有释放旧缓冲区（内存泄漏）、
// 漏写 return *this、以及返回类型写成 Hstring& 无法通过编译。
template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator+=(const Hstring& str) {
    return append(str);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator+=(const char* s) {
    return append(s);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator+=(char ch) {
    push_back(ch);
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::operator+=(std::initializer_list<char> ilist) {
    return append(ilist.begin(), ilist.end());
}

// 统一用 reserve() 完成扩容：它内部会拷贝旧内容并释放旧缓冲区。
// 注意默认实参只能写在头文件的声明处，定义处再写一遍是编译错误。
template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::append(const Hstring& str) {
    if(&str == this) {                    // 自别名：先复制再追加
        Hstring<Allocator> tmp(str);
        return append(tmp);
    }
    return append(str.data_, str.size_);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::append(const Hstring& str, size_type pos, size_type count) {
    if(pos > str.size_) {
        throw std::out_of_range("append: position out of range");
    }
    const size_type len = std::min(count, str.size_ - pos);
    return append(str.data_ + pos, len);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::append(const char* s, size_type count) {
    if(count == 0)    return *this;
    if(s == nullptr)  throw std::invalid_argument("append: null pointer");
    if(points_into(s, data_, size_)) {    // 自别名：扩容会让 s 悬空
        const Hstring tmp(s, count);
        return append(tmp.data(), count);
    }
    reserve(size_ + count + 1);
    std::char_traits<char>::copy(data_ + size_, s, count);
    size_ += count;
    data_[size_] = '\0';
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::append(const char* s) {
    if(s == nullptr)  return *this;
    return append(s, std::char_traits<char>::length(s));
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::append(size_type count, char ch) {
    if(count == 0)  return *this;
    reserve(size_ + count + 1);
    std::fill_n(data_ + size_, count, ch);
    size_ += count;
    data_[size_] = '\0';
    return *this;
}

template<typename Allocator>
void Hstring<Allocator>::push_back(char ch) {
    if(size_ + 2 > capacity_) {
        // 容量按倍增策略增长，使 push_back 均摊 O(1)。
        // 必须和 size_ + 2 取较大值：容量为 1 时 capacity_ * 2 只有 2，
        // 而写入 1 个字符加结尾 '\0' 需要 3 个槽位。
        reserve(std::max(capacity_ * 2, size_ + 2));
    }
    data_[size_] = ch;
    ++size_;
    data_[size_] = '\0';
}

template<typename Allocator>
void Hstring<Allocator>::pop_back() {
    if(size_ > 0) {
        --size_;
        data_[size_] = '\0';
    }
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::assign(const Hstring& str) {
    if(&str == this)  return *this;        // 自赋值
    return assign(str.data_, str.size_);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::assign(const char* s) {
    if(s == nullptr)  return *this;
    return assign(s, std::char_traits<char>::length(s));
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::assign(const char* s, size_type count) {
    if(count == 0) {
        size_ = 0;
        if(data_ != nullptr)  data_[0] = '\0';
        return *this;
    }
    if(s == nullptr)  throw std::invalid_argument("assign: null pointer");
    if(points_into(s, data_, size_)) {     // 自别名：扩容会让 s 悬空
        const Hstring tmp(s, count);
        return assign(tmp.data(), count);
    }
    if(count + 1 > capacity_)  reserve(count + 1);
    std::char_traits<char>::copy(data_, s, count);
    data_[count] = '\0';
    size_ = count;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::assign(size_type count, char ch) {
    if(count == 0) {
        size_ = 0;
        if(data_ != nullptr)  data_[0] = '\0';
        return *this;
    }
    if(count + 1 > capacity_)  reserve(count + 1);
    std::fill_n(data_, count, ch);
    data_[count] = '\0';
    size_ = count;
    return *this;
}

// 公共插入实现：在 pos 处腾出 count 个槽位，把原内容（含结尾 '\0'）整体后移。
// src 为 nullptr 时只腾空间，由调用者填充。
template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::insert_raw(size_type pos, const char* src, size_type count) {
    if(pos > size_) {
        throw std::out_of_range("insert: position out of range");
    }
    if(count == 0)  return *this;
    const size_type new_sz = size_ + count;
    reserve(new_sz + 1);
    // 目标区间在源区间右侧且相互重叠，必须反向搬移，否则会覆盖未搬走的字符。
    // 注意把结尾的 '\0' 也一起搬过去。
    std::move_backward(data_ + pos, data_ + size_ + 1, data_ + new_sz + 1);
    if(src != nullptr) {
        std::char_traits<char>::copy(data_ + pos, src, count);
    }
    size_ = new_sz;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::insert(size_type pos, const Hstring& str) {
    if(&str == this) {                     // 自插入：先复制，避免扩容后源失效
        Hstring<Allocator> tmp(str);
        return insert(pos, tmp);
    }
    return insert_raw(pos, str.data_, str.size_);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::insert(size_type pos, const char* s) {
    if(s == nullptr)  return insert_raw(pos, nullptr, 0);
    if(points_into(s, data_, size_)) {     // 自别名保护
        const Hstring tmp(s);
        return insert_raw(pos, tmp.data(), tmp.size());
    }
    return insert_raw(pos, s, std::char_traits<char>::length(s));
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::insert(size_type pos, size_type count, char ch) {
    insert_raw(pos, nullptr, count);
    if(count != 0)  std::fill_n(data_ + pos, count, ch);
    return *this;
}

template<typename Allocator>
typename Hstring<Allocator>::iterator Hstring<Allocator>::insert(const_iterator pos, char ch) {
    const size_type idx = index_of(pos);
    insert_raw(idx, nullptr, 1);
    data_[idx] = ch;
    return iterator(data_ + idx);
}

template<typename Allocator>
typename Hstring<Allocator>::iterator Hstring<Allocator>::insert(const_iterator pos, size_type count, char ch) {
    const size_type idx = index_of(pos);
    insert_raw(idx, nullptr, count);
    if(count != 0)  std::fill_n(data_ + idx, count, ch);
    return iterator(data_ + idx);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::erase(size_type pos, size_type count) {
    // 与 std::string 一致：pos == size_ 是合法的空操作，只有 pos > size_ 才抛异常。
    if(pos > size_) {
        throw std::out_of_range("erase: position out of range");
    }
    const size_type len = std::min(count, size_ - pos);
    if(len == 0)  return *this;
    // 连同结尾 '\0' 一起前移。前向移动，目标在源的左侧，安全。
    std::move(data_ + pos + len, data_ + size_ + 1, data_ + pos);
    size_ -= len;
    return *this;
}

template<typename Allocator>
typename Hstring<Allocator>::iterator Hstring<Allocator>::erase(const_iterator pos) {
    const size_type idx = index_of(pos);
    if(idx >= size_) {
        throw std::out_of_range("erase: iterator out of range");
    }
    std::move(data_ + idx + 1, data_ + size_ + 1, data_ + idx);
    --size_;
    return iterator(data_ + idx);
}

template<typename Allocator>
typename Hstring<Allocator>::iterator Hstring<Allocator>::erase(const_iterator first, const_iterator last) {
    const size_type i1 = index_of(first);
    const size_type i2 = index_of(last);
    if(i1 > size_ || i2 > size_ || i1 > i2) {
        throw std::out_of_range("erase: invalid range");
    }
    if(i1 == i2)  return iterator(data_ + i1);
    std::move(data_ + i2, data_ + size_ + 1, data_ + i1);
    size_ -= (i2 - i1);
    return iterator(data_ + i1);
}

// 公共替换实现：用 n 个字符替换 [pos, pos + len)。
// 原实现搬移时没有包含结尾的 '\0'，导致替换后字符串丢失终止符。
template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::replace_raw(size_type pos, size_type len, const char* src, size_type n) {
    const size_type new_sz = size_ - len + n;
    reserve(new_sz + 1);
    if(n < len) {
        // 区间变短：目标在源的左侧，前向搬移安全。
        std::move(data_ + pos + len, data_ + size_ + 1, data_ + pos + n);
    } else if(n > len) {
        // 区间变长：目标在源的右侧，必须反向搬移。
        std::move_backward(data_ + pos + len, data_ + size_ + 1, data_ + new_sz + 1);
    }
    if(n != 0 && src != nullptr) {
        std::char_traits<char>::copy(data_ + pos, src, n);
    }
    size_ = new_sz;
    return *this;
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::replace(size_type pos, size_type count, const Hstring& str) {
    if(pos > size_) {
        throw std::out_of_range("replace: position out of range");
    }
    if(&str == this) {                     // 自替换：先复制，避免搬移时源被覆盖
        Hstring<Allocator> tmp(str);
        return replace(pos, count, tmp);
    }
    const size_type len = std::min(count, size_ - pos);
    return replace_raw(pos, len, str.data_, str.size_);
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::replace(size_type pos, size_type count, const char* s) {
    if(pos > size_) {
        throw std::out_of_range("replace: position out of range");
    }
    if(s == nullptr) {
        const size_type len = std::min(count, size_ - pos);
        return replace_raw(pos, len, nullptr, 0);
    }
    if(points_into(s, data_, size_)) {     // 自别名保护
        const Hstring tmp(s);
        const size_type len = std::min(count, size_ - pos);
        return replace_raw(pos, len, tmp.data(), tmp.size());
    }
    const size_type len = std::min(count, size_ - pos);
    return replace_raw(pos, len, s, std::char_traits<char>::length(s));
}

template<typename Allocator>
Hstring<Allocator>& Hstring<Allocator>::replace(size_type pos, size_type count, size_type count2, char ch) {
    if(pos > size_) {
        throw std::out_of_range("replace: position out of range");
    }
    const size_type len = std::min(count, size_ - pos);
    replace_raw(pos, len, nullptr, count2);
    if(count2 != 0)  std::fill_n(data_ + pos, count2, ch);
    return *this;
}

template<typename Allocator>
void Hstring<Allocator>::swap(Hstring& other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

template<typename Allocator>
Hstring<Allocator> Hstring<Allocator>::substr(size_type pos, size_type count) const {
    // pos == size_ 合法（返回空串），只有 pos > size_ 才抛异常。
    if(pos > size_) {
        throw std::out_of_range("substr: position out of range");
    }
    const size_type len = std::min(count, size_ - pos);
    return Hstring<Allocator>(data_ + pos, len);
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::copy(char* dest, size_type count, size_type pos) const {
    if(pos > size_) {
        throw std::out_of_range("copy: position out of range");
    }
    const size_type len = std::min(count, size_ - pos);
    if(len != 0) {
        std::char_traits<char>::copy(dest, data_ + pos, len);
    }
    return len;
}

// 三路比较：先比公共前缀，前缀相同再比长度。
// 返回值的正负与 std::string::compare 一致（不保证恰好是 -1 / 0 / 1）。
template<typename Allocator>
int Hstring<Allocator>::compare(const Hstring& str) const {
    const size_type n = std::min(size_, str.size_);
    int r = 0;
    if(n != 0) {                    // 长度为 0 时不调用 compare，避免 nullptr 陷阱
        r = std::char_traits<char>::compare(data_, str.data_, n);
    }
    if(r != 0)  return r;
    if(size_ < str.size_)  return -1;
    if(size_ > str.size_)  return 1;
    return 0;
}

template<typename Allocator>
int Hstring<Allocator>::compare(const char* s) const {
    if(s == nullptr)  return size_ == 0 ? 0 : 1;
    const size_type n = std::char_traits<char>::length(s);
    const size_type m = std::min(size_, n);
    int r = 0;
    if(m != 0) {
        r = std::char_traits<char>::compare(data_, s, m);
    }
    if(r != 0)  return r;
    if(size_ < n)  return -1;
    if(size_ > n)  return 1;
    return 0;
}

template<typename Allocator>
int Hstring<Allocator>::compare(size_type pos, size_type count, const Hstring& str) const {
    if(pos > size_) {
        throw std::out_of_range("compare: position out of range");
    }
    const size_type len = std::min(count, size_ - pos);
    const size_type m = std::min(len, str.size_);
    int r = 0;
    if(m != 0) {
        r = std::char_traits<char>::compare(data_ + pos, str.data_, m);
    }
    if(r != 0)  return r;
    if(len < str.size_)  return -1;
    if(len > str.size_)  return 1;
    return 0;
}

// 注意：size_type 是类的成员 typedef，类外定义时返回类型必须写
// typename Hstring<Allocator>::size_type；同时默认实参只能在头文件中写一次。
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find(const Hstring& str, size_type pos) const {
    const size_type n = str.size_;
    if(n == 0)  return pos <= size_ ? pos : npos;   // 空子串总是“找得到”
    // 必须先判 pos > size_ 再算 size_ - pos，否则无符号下溢。
    // 同理 n > size_ - pos 时直接返回，避免下面的循环出现 size_ - len 下溢。
    if(pos > size_ || n > size_ - pos)  return npos;
    for(size_type i = pos; i + n <= size_; ++i) {
        if(std::char_traits<char>::compare(data_ + i, str.data_, n) == 0) {
            return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find(const char* s, size_type pos) const {
    if(s == nullptr)  return npos;
    const size_type n = std::char_traits<char>::length(s);
    if(n == 0)  return pos <= size_ ? pos : npos;
    if(pos > size_ || n > size_ - pos)  return npos;
    for(size_type i = pos; i + n <= size_; ++i) {
        if(std::char_traits<char>::compare(data_ + i, s, n) == 0) {
            return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find(char ch, size_type pos) const {
    for(size_type i = pos; i < size_; ++i) {
        if(data_[i] == ch) {
            return i;
        }
    }
    return npos;
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::rfind(const Hstring& str, size_type pos) const {
    const size_type n = str.size_;
    if(n > size_)  return npos;
    if(n == 0)  return std::min(pos, size_);   // 空子串：返回 min(pos, size_)
    const size_type start = std::min(pos, size_ - n);
    for(size_type i = start + 1; i-- > 0;) {
        if(std::char_traits<char>::compare(data_ + i, str.data_, n) == 0) {
            return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::rfind(const char* s, size_type pos) const {
    if(s == nullptr)  return npos;
    const size_type n = std::char_traits<char>::length(s);
    if(n > size_)  return npos;
    if(n == 0)  return std::min(pos, size_);
    const size_type start = std::min(pos, size_ - n);
    for(size_type i = start + 1; i-- > 0;) {
        if(std::char_traits<char>::compare(data_ + i, s, n) == 0) {
            return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::rfind(char ch, size_type pos) const {
    if(size_ == 0)  return npos;
    const size_type start = std::min(pos, size_ - 1);
    for(size_type i = start + 1; i-- > 0;) {
        if(data_[i] == ch) {
            return i;
        }
    }
    return npos;
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_first_of(const Hstring& str, size_type pos) const {
    for(size_type i = pos; i < size_; ++i) {
        if(str.find(data_[i]) != npos) {   // data_[i] 是否出现在 str 中
            return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_first_of(const char* s, size_type pos) const {
    if(s == nullptr)  return npos;
    for(size_type i = pos; i < size_; ++i) {
        for(const char* p = s; *p != '\0'; ++p) {
            if(*p == data_[i])  return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_first_of(char ch, size_type pos) const {
    return find(ch, pos);
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_last_of(const Hstring& str, size_type pos) const {
    if(size_ == 0)  return npos;
    const size_type start = std::min(pos, size_ - 1);
    for(size_type i = start + 1; i-- > 0;) {
        if(str.find(data_[i]) != npos) {
            return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_last_of(const char* s, size_type pos) const {
    // 同 find_first_of：原实现误用子串查找，应为逐字符成员判断。
    if(s == nullptr || size_ == 0)  return npos;
    const size_type start = std::min(pos, size_ - 1);
    for(size_type i = start + 1; i-- > 0;) {
        for(const char* p = s; *p != '\0'; ++p) {
            if(*p == data_[i])  return i;
        }
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_last_of(char ch, size_type pos) const {
    return rfind(ch, pos);
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_first_not_of(const Hstring& str, size_type pos) const {
    for(size_type i = pos; i < size_; ++i) {
        if(str.find(data_[i]) == npos) {   // data_[i] 不属于 str
            return i;
        }
    }
    return npos;
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_first_not_of(const char* s, size_type pos) const {
    for(size_type i = pos; i < size_; ++i) {
        bool belongs = false;
        if(s != nullptr) {
            for(const char* p = s; *p != '\0'; ++p) {
                if(*p == data_[i]) { belongs = true; break; }
            }
        }
        if(!belongs)  return i;
    }
    return npos;
}
template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_first_not_of(char ch, size_type pos) const {
    for(size_type i = pos; i < size_; ++i) {
        if(data_[i] != ch) {
            return i;
        }
    }
    return npos;
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_last_not_of(const Hstring& str, size_type pos) const {
    if(size_ == 0)  return npos;
    const size_type start = std::min(pos, size_ - 1);
    for(size_type i = start + 1; i-- > 0;) {
        if(str.find(data_[i]) == npos) {
            return i;
        }
    }
    return npos;
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_last_not_of(const char* s, size_type pos) const {
    if(size_ == 0)  return npos;
    const size_type start = std::min(pos, size_ - 1);
    for(size_type i = start + 1; i-- > 0;) {
        bool belongs = false;
        if(s != nullptr) {
            for(const char* p = s; *p != '\0'; ++p) {
                if(*p == data_[i]) { belongs = true; break; }
            }
        }
        if(!belongs)  return i;
    }
    return npos;
}

template<typename Allocator>
typename Hstring<Allocator>::size_type
Hstring<Allocator>::find_last_not_of(char ch, size_type pos) const {
    if(size_ == 0)  return npos;
    const size_type start = std::min(pos, size_ - 1);
    for(size_type i = start + 1; i-- > 0;) {
        if(data_[i] != ch) {
            return i;
        }
    }
    return npos;
}

// 比较运算符（==, !=, <, <=, >, >=）已改为头文件中的非成员函数模板。
// 它们不能是成员函数：成员函数的左操作数必须是本类对象，而这里需要支持
// "const char* == Hstring" 这类左操作数是 C 字符串的场景。

// 所有非模板成员都已有定义，因此可以整体实例化。
// 泛型迭代器构造函数是成员模板，不参与类级实例化，其定义在头文件中。
template class Hstring<std::allocator<char>>;
