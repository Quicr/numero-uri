add_executable(Testuint128 Testuint128.cc)

target_link_libraries(Testuint128 PUBLIC
numero_uri_lib
gtest_main
)

add_test(
    NAME Testuint128
    COMMAND Testuint128
)