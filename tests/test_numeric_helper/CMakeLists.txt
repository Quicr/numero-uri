add_executable(TestNumericHelper TestNumericHelper.cc)

target_link_libraries(TestNumericHelper PUBLIC
    numero_uri_lib
    gtest_main
)

add_test(
    NAME TestNumericHelper
    COMMAND TestNumericHelper
)