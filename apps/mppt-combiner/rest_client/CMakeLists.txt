add_library(rest_client INTERFACE)
target_sources(rest_client INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/rest_client.c
)
target_include_directories(rest_client INTERFACE ${CMAKE_CURRENT_LIST_DIR})