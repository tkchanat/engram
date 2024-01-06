cmake_minimum_required(VERSION 3.15)

add_executable(primitives primitives.cpp)
target_link_libraries(primitives PRIVATE engram)