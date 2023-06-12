cmake_minimum_required(VERSION 3.24)
project(engram)

set(CMAKE_CXX_STANDARD 17)
add_library(engram INTERFACE)
target_include_directories(engram INTERFACE .)

option(BUILD_EXAMPLES "Build example executables" ON)
if(BUILD_EXAMPLES)
  message("Building examples")
  add_subdirectory(examples)
endif()