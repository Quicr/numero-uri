find_package(Git QUIET)

# Check that git exists
if (NOT GIT_FOUND AND NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(FATAL_ERROR "Git doesn't exist or the directory is not a git repository")
endif()

option(GIT_SUBMODULE "Check submodules during build" ON)
if (GIT_SUBMODULE)

    # All submodules library

    add_subdirectory(json)
    add_subdirectory(libquicr)

    # googletests library
    if (BUILD_TESTING AND NUMERO_URI_BUILD_TESTS)
        add_subdirectory(googletest)
        # Prevent override the parent project's compiler/linker settings for windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
endif()
