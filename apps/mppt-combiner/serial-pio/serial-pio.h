#pragma once

#define SERIALPIO_RX_PIN 3
#define SERIALPIO_BAUD 9600
#define SERIALPIO_BUF_SIZE 256
#define HEARTBEAT_MS 1000
#define POLL_INTERVAL_MS 1
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t data[SERIALPIO_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint32_t dropped;
} serialpio_rx_buffer_t;

extern serialpio_rx_buffer_t rxbuf;
void run_serial_pio_monitor_task(void* argument);
void banger_task(void* argument);
bool try_process_line(char* line, char* key, char* value);
bool try_extract_line(char* buffer, char* extracted_line);
float to_scaled_float(const char* value, float scale);
