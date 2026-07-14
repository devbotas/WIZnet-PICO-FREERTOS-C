#include "blinker.h"
#include <stdio.h>
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

void blinker_task(void* param) {
    (void)param;

    gpio_init(USER_LED_GPIO);
    gpio_set_dir(USER_LED_GPIO, GPIO_OUT);

    bool led_on = false;

    while (true) {
        led_on = !led_on;
        gpio_put(USER_LED_GPIO, led_on);
        printf("LED %s\n", led_on ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
