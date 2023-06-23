add_executable(numero_uri_test TestUrlEncoder.cpp)

target_link_libraries(numero_uri_test PUBLIC
    numero_uri_lib
    gtest_main
)

add_executable(numero_uri_benchmark TestUrlEncoderPerformance.cpp)

target_link_libraries(numero_uri_benchmark PUBLIC
    numero_uri_lib
    gtest_main
)

include(GoogleTest)
gtest_add_tests(TARGET numero_uri_test)
gtest_add_tests(TARGET numero_uri_benchmark)