pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/uart_rx.pio)

target_sources(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/serial-pio.c
        ${CMAKE_CURRENT_LIST_DIR}/serial-pio-tasks.c
)
target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)