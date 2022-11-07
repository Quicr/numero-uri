# Fetch json for lib use
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_executable(webex_http_encoder main.cc)
set_target_properties(webex_http_encoder PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

add_library(webex_http_encoder_lib
    HttpEncoder.cc
    UrlTemplater.cc
)
set_target_properties(webex_http_encoder_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
target_link_libraries(webex_http_encoder_lib PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(webex_http_encoder_lib PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/inc")


target_link_libraries(webex_http_encoder PUBLIC
                      webex_http_encoder_lib)