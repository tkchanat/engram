cmake_minimum_required(VERSION 3.15)

add_executable(primitives primitives.cpp)
target_link_libraries(primitives PRIVATE engram)

add_executable(stl_types stl_types.cpp)
target_link_libraries(stl_types PRIVATE engram)

add_executable(file_io file_io.cpp)
target_link_libraries(file_io PRIVATE engram)