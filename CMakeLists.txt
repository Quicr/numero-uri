
cmake_minimum_required(VERSION 3.8)

# Build tests by default only if not a sub-project
if(DEFINED PROJECT_NAME)
    option(numero_uri_BUILD_TESTS "Build tests for qname" OFF)
else()
    option(numero_uri_BUILD_TESTS "Build tests for qname" ON)
endif()

project(numero_uri C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

add_subdirectory(contrib)
add_subdirectory(lib)

if (BUILD_TESTING AND numero_uri_BUILD_TESTS)
    add_subdirectory(cli)
    enable_testing()
    add_subdirectory(tests)
endif()
