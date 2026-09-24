cmake_minimum_required (VERSION 3.0)

project(Hstring)

add_executable(Hstring cpp/main.cpp cpp/Hstring.cpp)

target_include_directories(Hstring PRIVATE ${PROJECT_SOURCE_DIR})