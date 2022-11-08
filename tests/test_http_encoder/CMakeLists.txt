add_executable(TestHttpEncoder TestHttpEncoder.cc)

target_link_libraries(TestHttpEncoder PUBLIC
    webex_http_encoder_lib
    gtest_main
)

add_test(
    NAME TestHttpEncoder
    COMMAND TestHttpEncoder
)