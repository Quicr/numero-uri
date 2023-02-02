add_library(numero_uri_lib
    UrlEncoder.cpp
)
set_target_properties(numero_uri_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
    "${PROJECT_BINARY_DIR}/lib")
target_link_libraries(numero_uri_lib 
    PUBLIC
        nlohmann_json
        quicr
)

target_include_directories(numero_uri_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
