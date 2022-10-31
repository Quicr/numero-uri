set (Sources
# main.cc
HttpEncoder.cc
UrlTemplater.cc
)

add_library(webex_http_encoder STATIC ${Sources})

target_include_directories(webex_http_encoder PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>)