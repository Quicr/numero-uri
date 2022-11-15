# Fetch json for lib use
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_executable(numero_uri main.cc)
set_target_properties(numero_uri PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_library(numero_uri_lib
    HttpEncoder.cc
    UrlTemplater.cc
)
set_target_properties(numero_uri_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
target_link_libraries(numero_uri_lib PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(numero_uri_lib PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/inc")


target_link_libraries(numero_uri PUBLIC
                      numero_uri_lib)