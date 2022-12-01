add_executable(Testbig_uint Testbig_uint.cc)

target_link_libraries(Testbig_uint PUBLIC
numero_uri_lib
gtest_main
)

add_test(
    NAME Testbig_uint
    COMMAND Testbig_uint
)