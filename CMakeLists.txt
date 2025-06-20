cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(LibT9)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_LIB "Build only the lib directory" OFF)

if(BUILD_LIB)
    add_subdirectory(src)
else()
    add_subdirectory(src)
    add_subdirectory(cli)
endif()
