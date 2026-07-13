set(TARGET_NAME mppt-combiner)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
        network.h
        network.c
        network-tasks.c
        blinker.h
        blinker.c
        system.h
        system.c
        mppt.h
        mppt.c
        serial-pio.h
        serial-pio.c
        serial-pio-tasks.c
)

target_include_directories(${TARGET_NAME} PUBLIC
        ${PORT_DIR}/FreeRTOS-Kernel/inc
)

pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/uart_rx.pio)


target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        hardware_pio
        FreeRTOS-Kernel-Heap4
        ETHERNET_FILES
        IOLIBRARY_FILES
        DHCP_FILES
        DNS_FILES
        TIMER_FILES
)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
