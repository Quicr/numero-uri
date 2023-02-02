add_executable(TestUrlEncoder TestUrlEncoder.cpp)

target_link_libraries(TestUrlEncoder PUBLIC
    numero_uri_lib
    gtest_main
)

add_test(
    NAME TestUrlEncoder
    COMMAND TestUrlEncoder
)

add_executable(TestUrlEncoderPerformance TestUrlEncoderPerformance.cpp)

target_link_libraries(TestUrlEncoderPerformance PUBLIC
    numero_uri_lib
    gtest_main
)

add_test(
    NAME TestUrlEncoderPerformance
    COMMAND TestUrlEncoderPerformance
)