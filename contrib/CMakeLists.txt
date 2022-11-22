find_package(Git QUIET)

# Check that git exists
if (NOT GIT_FOUND AND NOT EXISTS "${PROJECT_SOURCE_DIR}/.git")
    message(FATAL_ERROR "Git doesn't exist or the directory is not a git repository")
endif()

option(GIT_SUBMODULE "Check submodules during build" ON)
if (GIT_SUBMODULE)

    # JSON library
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule
        update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/json
        RESULT_VARIABLE JSON_SUBMOD_RES)
    if (NOT JSON_SUBMOD_RES EQUAL "0")
        message(FATAL_ERROR "git module update --init --recursive failed with ${JSON_SUBMOD_RES}, please checkout submodules")
    endif()
    add_subdirectory(json)

    # googletests library
    if (BUILD_TESTS)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule
            update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/googletest
            RESULT_VARIABLE JSON_SUBMOD_RES)
        if (NOT JSON_SUBMOD_RES EQUAL "0")
            message(FATAL_ERROR "git module update --init --recursive failed with ${JSON_SUBMOD_RES}, please checkout submodules")
        endif()
        add_subdirectory(googletest)
        # Prevent override the parent project's compiler/linker settings for windows
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

endif()
