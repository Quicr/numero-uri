add_executable(Testuint128 Testuint128.cc)

target_link_libraries(Testuint128 PUBLIC
webex_http_encoder_lib
gtest_main
)

add_test(
    NAME Testuint128
    COMMAND Testuint128
)