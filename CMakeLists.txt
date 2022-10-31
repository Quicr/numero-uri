
cmake_minimum_required(VERSION 3.8)
set(This webex_http_encoder)
project(${This} C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

# Add subdirectory goes into the directory and looks for a cmakelists file
add_subdirectory(src)

# TODO add a condition
enable_testing()
add_subdirectory(tests)