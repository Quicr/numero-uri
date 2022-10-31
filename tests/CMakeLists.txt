# Fetch GTest for lib use
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         e2239ee6043f73722e7aa812a459f54a28552929 # 1.11.0
)

# Prevent override the parent project's compiler/linker settings for windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(HttpEncoderTest HttpEncoderTest.cc)

target_link_libraries(HttpEncoderTest PUBLIC
webex_http_encoder
gtest_main
)

add_test(
    NAME HttpEncoderTest
    COMMAND HttpEncoderTest
)