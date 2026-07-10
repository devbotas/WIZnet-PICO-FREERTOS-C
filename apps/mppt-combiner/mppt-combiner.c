#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include "port_common.h"

#include "network.h"
#include "blinker.h"
#include "system.h"

int main()
{
    set_clock_khz();

    stdio_init_all();
    sleep_ms(3000);

    network_init();

    xTaskCreate(get_ipi_from_dhcp_task, "DHCP", 2048, NULL, 8, NULL);
    xTaskCreate(blinker_task, "blinker", 256, NULL, 1, NULL);

    dns_semaphore = xSemaphoreCreateCounting((unsigned portBASE_TYPE)0x7fffffff, (unsigned portBASE_TYPE)0);

    vTaskStartScheduler();

    // Should never reach this.
    while (1)
    {
    }
}
