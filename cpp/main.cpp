#include <cassert>
#include <iostream>
#include <string>
#include <utility>
#include "hpp/Hstring.hpp"

using String = Hstring<>;

static void expect(const String& value, const char* expected) {
    assert(value.size() == std::char_traits<char>::length(expected));
    assert(value.size() == 0 || value.capacity() >= value.size() + 1);
    assert(std::string(value.c_str()) == expected);
    assert(std::string(value.begin(), value.end()) == expected);
    assert(value.data() != nullptr || value.size() == 0);
}

int main() {
    String empty;
    expect(empty, "");

    String from_cstr("hello");
    String null_string(nullptr);
    expect(from_cstr, "hello");
    expect(null_string, "");

    String prefix("hello", 2);
    String exact("hello", 5);
    String oversized("hello", 20);
    expect(prefix, "he");
    expect(exact, "hello");
    expect(oversized, "hello");

    String repeated(4, 'x');
    String empty_repeated(0, 'x');
    String listed{'a', 'b', 'c'};
    expect(repeated, "xxxx");
    expect(empty_repeated, "");
    expect(listed, "abc");

    const char source[] = "abcdef";
    String pointer_range(source, source + 3);
    std::string standard_string = "wxyz";
    String container_range(standard_string.begin(), standard_string.end());
    expect(pointer_range, "abc");
    expect(container_range, "wxyz");

    String copied(from_cstr);
    String copy_assigned;
    copy_assigned = from_cstr;
    copy_assigned = copy_assigned;
    expect(copied, "hello");
    expect(copy_assigned, "hello");

    String move_source("move me");
    String moved(std::move(move_source));
    expect(moved, "move me");
    expect(move_source, "");

    String move_assigned;
    move_assigned = std::move(moved);
    expect(move_assigned, "move me");
    expect(moved, "");

    String assigned("long value");
    assigned = "short";
    expect(assigned, "short");
    assigned = "a much longer value";
    expect(assigned, "a much longer value");
    assigned = 'Z';
    expect(assigned, "Z");
    assigned = {'u', 'v', 'w'};
    expect(assigned, "uvw");
    assigned = {};
    expect(assigned, "");

    std::cout << "All Hstring tests passed.\n";
}