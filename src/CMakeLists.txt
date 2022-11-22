add_executable(numero_uri main.cc)
set_target_properties(numero_uri PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${PROJECT_BINARY_DIR}/bin")

add_library(numero_uri_lib
    HttpEncoder.cc
    UrlTemplater.cc
)
set_target_properties(numero_uri_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
    "${PROJECT_BINARY_DIR}/lib")
target_link_libraries(numero_uri_lib PUBLIC nlohmann_json)

target_include_directories(numero_uri_lib PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/inc",
"${PROJECT_SOURCE_DIR}/inc_private")

target_include_directories(numero_uri_lib PRIVATE
"${PROJECT_SOURCE_DIR}/inc_private")

target_link_libraries(numero_uri PUBLIC
                      numero_uri_lib)