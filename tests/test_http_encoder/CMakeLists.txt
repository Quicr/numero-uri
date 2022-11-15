add_executable(TestHttpEncoder TestHttpEncoder.cc)

target_link_libraries(TestHttpEncoder PUBLIC
    numero_uri_lib
    gtest_main
)

add_test(
    NAME TestHttpEncoder
    COMMAND TestHttpEncoder
)