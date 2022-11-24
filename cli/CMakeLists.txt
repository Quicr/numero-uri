add_executable(numero_uri
    src/main.cc
)

set_target_properties(numero_uri PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${PROJECT_BINARY_DIR}/bin")

target_include_directories(numero_uri PRIVATE
    inc
)

target_link_libraries(numero_uri PUBLIC
                      numero_uri_lib)