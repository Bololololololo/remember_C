cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Add your static library
# Adjust the path to where libexp.a is located
add_library(libbolloc STATIC IMPORTED)
set_target_properties(libbolloc PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../libbolloc/build/libbolloc.a
)

# Include directory for library headers (adjust path as needed)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libbolloc/include)

add_executable(
  unit_tests
  unit_tests.cc
)
target_link_libraries(
  unit_tests
  GTest::gtest_main
  libbolloc
)

#include(GoogleTest)
#gtest_discover_tests(unit_tests)
