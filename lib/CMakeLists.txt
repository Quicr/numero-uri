add_library(numero_uri_lib
    src/UrlEncoder.cpp
    inc/UrlEncoder.h
)
set_target_properties(numero_uri_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
    "${PROJECT_BINARY_DIR}/lib")

target_link_libraries(numero_uri_lib 
    PUBLIC
        quicr
        nlohmann_json
)

target_include_directories(numero_uri_lib PUBLIC ${PROJECT_BINARY_DIR} inc)
