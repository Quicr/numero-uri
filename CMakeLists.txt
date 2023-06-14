
cmake_minimum_required(VERSION 3.8)
project(numero_uri C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

option(NUMERO_URI_BUILD_TESTS "build the tests" OFF)

add_subdirectory(contrib)
add_subdirectory(lib)

if (BUILD_TESTING AND NUMERO_URI_BUILD_TESTS)
    add_subdirectory(cli)
    enable_testing()
    add_subdirectory(tests)
endif()
