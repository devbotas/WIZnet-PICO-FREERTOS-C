target_sources(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/network.c
        ${CMAKE_CURRENT_LIST_DIR}/network-tasks.c
)
target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
