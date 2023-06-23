if (NOT TARGET nlohmann_json)
    add_subdirectory(json)
endif()
if (NOT TARGET qname)
    add_subdirectory(qname)
endif()

if (BUILD_TESTING AND numero_uri_BUILD_TESTS)
    add_subdirectory(googletest)
    # Prevent override the parent project's compiler/linker settings for windows
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
