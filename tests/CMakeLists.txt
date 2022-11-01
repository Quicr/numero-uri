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

add_subdirectory(test_http_encoder)
add_subdirectory(test_uint128)