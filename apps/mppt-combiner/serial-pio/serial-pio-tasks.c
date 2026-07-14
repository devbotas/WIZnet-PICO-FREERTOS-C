#include "serial-pio.h"

#include "../network/network.h"
#include "FreeRTOS.h"
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <uart_rx.pio.h>

#include "helpers.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "pico/time.h"
#include "mppt.h"


static inline void serialpio_begin(PIO pio, uint sm, uint offset, uint pin, uint baud) {
    uart_rx_program_init(pio, sm, offset, pin, baud);
}


void run_serial_pio_monitor_task(void* argument) {
    sleep_ms(1500);

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &uart_rx_program);

    serialpio_begin(pio, sm, offset, SERIALPIO_RX_PIN, SERIALPIO_BAUD);

    printf("PIO Serial RX on GPIO %u at %u baud\n", SERIALPIO_RX_PIN, SERIALPIO_BAUD);
    printf("Connect an external UART TX signal to GPIO %u\n", SERIALPIO_RX_PIN);

    absolute_time_t next_heartbeat = delayed_by_ms(get_absolute_time(), HEARTBEAT_MS);
    char frame[1000] = {0};
    char last_line[256] = {0};
    size_t frame_length = 0;
    size_t line_length = 0;
    char bybiline[256] = {0};

    while (true) {
        while (!pio_sm_is_rx_fifo_empty(pio, sm)) {
            char c = uart_rx_program_getc(pio, sm);

            if (c == '\n') {
                last_line[line_length] = '\0';

                char key[50], value[50];
                if (try_process_line(last_line, key, value)) {
                    //printf("Key: %s, value: %s\n", key, value);

                    if (strcmp(key, "Checksum") == 0) {
                        int actual_checksum_value = 0;
                        for (int i = 0; i < frame_length; i++) {
                            actual_checksum_value = (actual_checksum_value + frame[i]) % 0xFF;
                        }

                        char sent_checksum_value = value[0];

                        while (try_extract_line(frame, bybiline)) {
                            if (try_process_line(bybiline, key, value)) {
                                printf("Key: %s, value: %s\n", key, value);
                            }
                        }

                        if (actual_checksum_value == sent_checksum_value) {
                            printf("pyzda!\n");
                        }
                        else {
                            printf("bybis\n");
                        }


                        frame_length = 0;
                    }
                }

                line_length = 0;
            }

            last_line[line_length] = c;
            frame[frame_length] = c;
            frame_length++;
            line_length++;
            // TODO: Add buffer overflow protection
        }

        sleep_ms(POLL_INTERVAL_MS);
    }
}

void banger_task(void* argument) {
    sleep_ms(3500);

    // Set up the hard UART we're going to use to print characters
    uart_init(uart1, 9600);
    gpio_set_function(4, GPIO_FUNC_UART);
    const char* bybis = "\n V\t20.0\n  \n Checksum \t 1";
    while (true) {
        uart_puts(uart1, bybis);

        sleep_ms(1000);
    }
}
