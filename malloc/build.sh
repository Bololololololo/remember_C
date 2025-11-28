#!/bin/bash

# cleanup
echo "# Cleaning up build directory"
rm -rf libbolloc/build
rm -rf unit_tests/build

# make build directory for the library
echo "# Creating build directory for the library"
cmake -S libbolloc -B libbolloc/build

# build library
echo "# Compiling library"
cd libbolloc/build && make

# go back to root
cd ../../

# make build directory for the gtest
echo "# Creating build directory for the gtests"
cmake -S unit_tests -B unit_tests/build -DCMAKE_BUILD_TYPE=Debug

# build project
echo "# Compiling project"
cd unit_tests/build && make

# lunch tests
# echo "# Launching tests"
# ./unit_tests

# go back to root
cd ..
