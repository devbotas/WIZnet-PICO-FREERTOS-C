set(TARGET_NAME mppt-combiner)

add_executable(${TARGET_NAME} ${TARGET_NAME}.c
        helpers/helpers.h
        helpers/helpers.c)

add_subdirectory(blinker)
add_subdirectory(helpers)
add_subdirectory(mppt)
add_subdirectory(network)
add_subdirectory(serial-pio)
add_subdirectory(system)

target_include_directories(${TARGET_NAME} PUBLIC
        ${PORT_DIR}/FreeRTOS-Kernel/inc
)

target_link_libraries(${TARGET_NAME} PRIVATE
        helpers
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
