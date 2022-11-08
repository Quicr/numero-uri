add_executable(TestNumericHelper TestNumericHelper.cc)

target_link_libraries(TestNumericHelper PUBLIC
    webex_http_encoder_lib
    gtest_main
)

add_test(
    NAME TestNumericHelper
    COMMAND TestNumericHelper
)