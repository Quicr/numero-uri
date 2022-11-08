add_executable(TestUrlTemplater TestUrlTemplater.cc)

target_link_libraries(TestUrlTemplater PUBLIC
    webex_http_encoder_lib
    gtest_main
)

add_test(
    NAME TestUrlTemplater
    COMMAND TestUrlTemplater
)
