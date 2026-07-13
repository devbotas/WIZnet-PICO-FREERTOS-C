#include "serial-pio.h"

#include "network.h"
#include "FreeRTOS.h"
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <uart_rx.pio.h>

#include "hardware/pio.h"
#include "hardware/uart.h"
#include "pico/time.h"


static inline void serialpio_begin(PIO pio, uint sm, uint offset, uint pin, uint baud)
{
    uart_rx_program_init(pio, sm, offset, pin, baud);
}

static inline bool serialpio_buffer_empty(void)
{
    return rxbuf.head == rxbuf.tail;
}

static inline bool serialpio_buffer_full(void)
{
    return ((rxbuf.head + 1u) % SERIALPIO_BUF_SIZE) == rxbuf.tail;
}

static inline void serialpio_buffer_push(uint8_t c)
{
    uint16_t next = (rxbuf.head + 1u) % SERIALPIO_BUF_SIZE;
    if (next == rxbuf.tail)
    {
        rxbuf.dropped++;
        return;
    }
    rxbuf.data[rxbuf.head] = c;
    rxbuf.head = next;
}

static inline int serialpio_read(void)
{
    if (serialpio_buffer_empty())
    {
        return -1;
    }
    uint8_t c = rxbuf.data[rxbuf.tail];
    rxbuf.tail = (rxbuf.tail + 1u) % SERIALPIO_BUF_SIZE;
    return c;
}

static inline uint16_t serialpio_available(void)
{
    if (rxbuf.head >= rxbuf.tail)
    {
        return rxbuf.head - rxbuf.tail;
    }
    return SERIALPIO_BUF_SIZE - rxbuf.tail + rxbuf.head;
}

static void serialpio_poll(PIO pio, uint sm)
{
    while (!pio_sm_is_rx_fifo_empty(pio, sm))
    {
        uint8_t c = (uint8_t)uart_rx_program_getc(pio, sm);
        serialpio_buffer_push(c);
    }
}

void run_serial_pio_monitor_task(void* argument)
{
    sleep_ms(1500);

    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &uart_rx_program);

    serialpio_begin(pio, sm, offset, SERIALPIO_RX_PIN, SERIALPIO_BAUD);

    printf("PIO Serial RX on GPIO %u at %u baud\n", SERIALPIO_RX_PIN, SERIALPIO_BAUD);
    printf("Connect an external UART TX signal to GPIO %u\n", SERIALPIO_RX_PIN);

    absolute_time_t next_heartbeat = delayed_by_ms(get_absolute_time(), HEARTBEAT_MS);
    char line[80];
    size_t line_len = 0;

    while (true)
    {
        serialpio_poll(pio, sm);

        while (serialpio_available() > 0)
        {
            int ch = serialpio_read();
            if (ch < 0)
            {
                break;
            }

            //printf("%c", ch);
            putchar(ch);

            sleep_ms(POLL_INTERVAL_MS);
        }
    }
}

void banger_task(void* argument)
{
    sleep_ms(3500);

    // Set up the hard UART we're going to use to print characters
    uart_init(uart1, 9600);
    gpio_set_function(4, GPIO_FUNC_UART);
    const char* bybis = "pyzda";
    while (true)
    {
        uart_puts(uart1, bybis);

        sleep_ms(1000);
    }
}
