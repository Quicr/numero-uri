add_executable(TestUrlTemplater TestUrlTemplater.cc)

target_link_libraries(TestUrlTemplater PUBLIC
    numero_uri_lib
    gtest_main
)

add_test(
    NAME TestUrlTemplater
    COMMAND TestUrlTemplater
)
