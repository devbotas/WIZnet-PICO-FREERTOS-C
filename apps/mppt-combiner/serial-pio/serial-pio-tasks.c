#include "serial-pio.h"

#include "../network/network.h"
#include "FreeRTOS.h"
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

                        mppt_data new_data = {0};
                        while (try_extract_line(frame, bybiline)) {
                            if (try_process_line(bybiline, key, value) == false) {
                                continue;
                            }

                            if (strcmp(key, "PID") == 0) {
                                snprintf(new_data.productId, sizeof(new_data.productId), "%s", value);
                            }
                            else if (strcmp(key, "FW") == 0) {
                                if (strlen(value) >= 3) {
                                    snprintf(new_data.firmwareVersion, sizeof(new_data.firmwareVersion), "%c.%.2s",
                                             value[0], value + 1);
                                }
                                else {
                                    snprintf(new_data.firmwareVersion, sizeof(new_data.firmwareVersion), "%s", value);
                                }
                            }
                            else if (strcmp(key, "SER#") == 0) {
                                snprintf(new_data.serialNumber, sizeof(new_data.serialNumber), "%s", value);
                            }
                            else if (strcmp(key, "V") == 0) {
                                new_data.batteryVoltageV = toScaledFloat(value, 1000.0f);
                            }
                            else if (strcmp(key, "I") == 0) {
                                new_data.batteryCurrentA = toScaledFloat(value, 1000.0f);
                            }
                            else if (strcmp(key, "VPV") == 0) {
                                new_data.panelVoltageV = toScaledFloat(value, 1000.0f);
                            }
                            else if (strcmp(key, "PPV") == 0) {
                                new_data.panelPowerW = (int)strtol(value, NULL, 10);
                            }
                            else if (strcmp(key, "CS") == 0) {
                                // For simplicity here, we could add a mapState helper to a shared header if needed,
                                // but for now let's just copy what mppt.c does or keep it simple.
                                // Actually, we should probably use the same mapping.
                                if (strcmp(value, "0") == 0)
                                    snprintf(new_data.stateText, sizeof(new_data.stateText),
                                             "Off");
                                else if (strcmp(value, "2") == 0)
                                    snprintf(
                                        new_data.stateText, sizeof(new_data.stateText), "Fault");
                                else if (strcmp(value, "3") == 0)
                                    snprintf(
                                        new_data.stateText, sizeof(new_data.stateText), "Bulk");
                                else if (strcmp(value, "4") == 0)
                                    snprintf(
                                        new_data.stateText, sizeof(new_data.stateText), "Absorption");
                                else if (strcmp(value, "5") == 0)
                                    snprintf(
                                        new_data.stateText, sizeof(new_data.stateText), "Float");
                                else if (strcmp(value, "7") == 0)
                                    snprintf(
                                        new_data.stateText, sizeof(new_data.stateText), "Equalize");
                                else snprintf(new_data.stateText, sizeof(new_data.stateText), "%s", value);
                            }
                            else if (strcmp(key, "ERR") == 0) {
                                new_data.errorCode = (int)strtol(value, NULL, 10);
                            }
                            else if (strcmp(key, "H20") == 0) {
                                new_data.yieldTodayKWh = toScaledFloat(value, 100.0f);
                            }
                            else if (strcmp(key, "H22") == 0) {
                                new_data.yieldYesterdayKWh = toScaledFloat(value, 100.0f);
                            }
                            else if (strcmp(key, "H21") == 0) {
                                new_data.maxPowerTodayW = (int)strtol(value, NULL, 10);
                            }
                            else if (strcmp(key, "H23") == 0) {
                                new_data.maxPowerYesterdayW = (int)strtol(value, NULL, 10);
                            }
                            else if (strcmp(key, "H19") == 0) {
                                new_data.yieldTotalKWh = toScaledFloat(value, 100.0f);
                            }
                            else if (strcmp(key, "HSDS") == 0) {
                                new_data.daySequenceNumber = (int)strtol(value, NULL, 10);
                            }
                            else if (strcmp(key, "IL") == 0) {
                                new_data.loadCurrentA = toScaledFloat(value, 1000.0f);
                            }
                            else if (strcmp(key, "LOAD") == 0) {
                                new_data.loadOutputState = strcmp(value, "ON") == 0 || strcmp(value, "1") == 0;
                            }
                            else if (strcmp(key, "MPPT") == 0) {
                                new_data.chargerModeId = (int)strtol(value, NULL, 10);
                            }

                            printf("Key: %s, value: %s\n", key, value);
                        }
                        new_data.deviceInstance = 256;
                        new_data.frameValid = true;
                        new_data.lastUpdateMs = to_ms_since_boot(get_absolute_time());

                        if (actual_checksum_value == sent_checksum_value) {
                            printf("pyzda!\n");
                        }
                        else {
                            printf("bybis\n");
                        }

                        xQueueSend(received_mppt_datas, &new_data, 0);

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
