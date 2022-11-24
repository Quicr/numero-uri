
cmake_minimum_required(VERSION 3.8)
project(numero_uri C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

option(BUILD_TESTS "build the tests" OFF)

# Add subdirectory goes into the directory and looks for a cmakelists file
add_subdirectory(contrib)
add_subdirectory(lib)
add_subdirectory(cli)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TESTS)