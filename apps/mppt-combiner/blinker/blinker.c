#include "blinker.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

void blinker_task(void* param) {
    (void)param;

    int blinker_gpio = 28;

    gpio_init(blinker_gpio);
    gpio_set_dir(blinker_gpio, GPIO_OUT);

    bool led_on = false;

    while (true) {
        led_on = !led_on;
        gpio_put(blinker_gpio, led_on);
        //printf("LED %s\n", led_on ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
