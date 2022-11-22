add_executable(TestUrlEncoder TestUrlEncoder.cc)

target_link_libraries(TestUrlEncoder PUBLIC
    numero_uri_lib
    gtest_main
)

add_test(
    NAME TestUrlEncoder
    COMMAND TestUrlEncoder
)