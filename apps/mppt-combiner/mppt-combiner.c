#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "mppt.h"
#include "port_common.h"

#include "network/network.h"
#include "blinker/blinker.h"
#include "serial-pio/serial-pio.h"
#include "system/system.h"

QueueHandle_t received_mppt_datas = NULL;

int main() {
    set_clock_khz();

    stdio_init_all();
    sleep_ms(3000);

    network_init();

    received_mppt_datas = xQueueCreate(10, sizeof(mppt_data_t));

    xTaskCreate(get_ipi_from_dhcp_task, "DHCP", 2048, NULL, 8, NULL);
    xTaskCreate(blinker_task, "blinker", 256, NULL, 1, NULL);

    static void* task_args_1[3];
    static uint32_t device_instance1 = 256;
    static uint8_t pin_number1 = 3;
    task_args_1[0] = received_mppt_datas;
    task_args_1[1] = &device_instance1;
    task_args_1[2] = &pin_number1;
    xTaskCreate(run_serial_pio_monitor_task, "pio1", 2560, task_args_1, 1, NULL);

    static void* task_args_2[3];
    static uint32_t device_instance2 = 257;
    static uint8_t pin_number2 = 8;
    task_args_2[0] = received_mppt_datas;
    task_args_2[1] = &device_instance2;
    task_args_2[2] = &pin_number2;
    xTaskCreate(run_serial_pio_monitor_task, "pio2", 2560, task_args_2, 1, NULL);

    xTaskCreate(banger_task, "banger", 256, NULL, 1, NULL);
    xTaskCreate(post_rest_continuously_task, "rest_poster", 1024, received_mppt_datas, 1, NULL);

    dns_semaphore = xSemaphoreCreateCounting((unsigned portBASE_TYPE)0x7fffffff, (unsigned portBASE_TYPE)0);

    vTaskStartScheduler();

    // Should never reach this.
    while (1) {
    }
}
