target_sources(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/mppt.c
)
target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
