cmake_minimum_required (VERSION 3.0)

project (json_parser VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

file (GLOB PARSER_SOURCES src/* main.cpp)

add_executable (json_parser ${PARSER_SOURCES})

install (TARGETS json_parser DESTINATION bin)