
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"

#include "dhcp.h"
#include "dns.h"

#include "timer.h"

#include "network.h"
#include "blinker.h"

#define PLL_SYS_KHZ (133 * 1000)
#define SOCKET_DNS 3
#define DNS_RETRY_COUNT 5

static uint8_t g_dns_target_domain[] = "www.wiznet.io";
static uint8_t g_dns_target_ip[4] = {    0,};

static volatile uint32_t g_msec_cnt = 0;
void dns_task(void* argument);
static void set_clock_khz(void);
static void repeating_timer_callback(void);

int main()
{
    set_clock_khz();

    stdio_init_all();
    sleep_ms(3000);

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    xTaskCreate(get_ipi_from_dhcp_task, "DHCP_Task", 2048, NULL, 8, NULL);
    xTaskCreate(dns_task, "DNS_Task", 512, NULL, 10, NULL);
    xTaskCreate(blinker_task, "blinker", 256, NULL, 1, NULL);
    dns_semaphore = xSemaphoreCreateCounting((unsigned portBASE_TYPE)0x7fffffff, (unsigned portBASE_TYPE)0);

    vTaskStartScheduler();

    while (1)
    {
    }
}

void dns_task(void* argument)
{
    uint8_t dns_retry;

    while (1)
    {
        xSemaphoreTake(dns_semaphore, portMAX_DELAY);
        DNS_init(SOCKET_DNS, ethernet_buffer);

        dns_retry = 0;

        while (1)
        {
            if (DNS_run(network_information.dns, g_dns_target_domain, g_dns_target_ip) > 0)
            {
                printf(" DNS success\n");
                printf(" Target domain : %s\n", g_dns_target_domain);
                printf(" IP of target domain : %d.%d.%d.%d\n", g_dns_target_ip[0], g_dns_target_ip[1],
                       g_dns_target_ip[2], g_dns_target_ip[3]);

                break;
            }
            else
            {
                dns_retry++;

                if (dns_retry <= DNS_RETRY_COUNT)
                {
                    printf(" DNS timeout occurred and retry %d\n", dns_retry);
                }
            }

            if (dns_retry > DNS_RETRY_COUNT)
            {
                printf(" DNS failed\n");

                break;
            }

            vTaskDelay(10);
        }
    }
}

static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0, // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000, // Input frequency
        PLL_SYS_KHZ * 1000 // Output (must be same as no divider)
    );
}

static void repeating_timer_callback(void)
{
    g_msec_cnt++;

    if (g_msec_cnt >= 1000 - 1)
    {
        g_msec_cnt = 0;

        DHCP_time_handler();
        DNS_time_handler();
    }
}
