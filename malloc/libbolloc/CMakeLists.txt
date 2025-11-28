cmake_minimum_required(VERSION 3.14)
project(bolloc C)

# Set debug flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(bolloc STATIC bolloc.c)