add_library(helpers INTERFACE)
target_sources(helpers PUBLIC ${CMAKE_CURRENT_LIST_DIR}/helpers.c)
target_include_directories(helpers INTERFACE ${CMAKE_CURRENT_LIST_DIR})