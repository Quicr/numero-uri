if (NOT TARGET nlohmann_json)
    add_subdirectory(json)
endif()
if (NOT TARGET quicr)
    add_subdirectory(libquicr)
endif()

if (BUILD_TESTING AND NUMERO_URI_BUILD_TESTS)
    add_subdirectory(googletest)
    # Prevent override the parent project's compiler/linker settings for windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
