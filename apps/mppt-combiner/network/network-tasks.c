#include "network.h"
#include "FreeRTOS.h"
#include <semphr.h>
#include <stdint.h>
#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include <stdio.h>
#include "dhcp.h"
#include "hardware/gpio.h"

void get_ipi_from_dhcp_task(void* argument) {
    int retval = 0;
    uint8_t link;
    uint16_t len = 0;
    uint32_t dhcp_retry = 0;

    int ip_blinker_gpio = 2;
    gpio_init(ip_blinker_gpio);
    gpio_set_dir(ip_blinker_gpio, GPIO_OUT);


    if (network_information.dhcp == NETINFO_DHCP) {
        initialize_dhcp();
    }
    else {
        // static
        network_initialize(network_information);

        /* Get network information */
        print_network_information(network_information);

        while (1) {
            vTaskDelay(1000 * 1000);
        }
    }

    while (1) {
        link = wizphy_getphylink();

        if (link == PHY_LINK_OFF) {
            gpio_put(ip_blinker_gpio,false);
            printf("PHY_LINK_OFF\n");

            DHCP_stop();

            while (1) {
                link = wizphy_getphylink();

                if (link == PHY_LINK_ON) {
                    initialize_dhcp();

                    dhcp_retry = 0;

                    break;
                }

                vTaskDelay(1000);
            }
        }

        retval = DHCP_run();

        if (retval == DHCP_IP_LEASED) {
            if (g_dhcp_get_ip_flag == 0) {
                dhcp_retry = 0;

                gpio_put(ip_blinker_gpio,true);
                printf(" DHCP success\n");

                g_dhcp_get_ip_flag = 1;

                xSemaphoreGive(dns_semaphore);
            }
        }
        else if (retval == DHCP_FAILED) {
            g_dhcp_get_ip_flag = 0;
            dhcp_retry++;

            if (dhcp_retry <= DHCP_RETRY_COUNT) {
                printf(" DHCP timeout occurred and retry %d\n", dhcp_retry);
            }
        }

        if (dhcp_retry > DHCP_RETRY_COUNT) {
            printf(" DHCP failed\n");

            DHCP_stop();

            while (1) {
                vTaskDelay(1000 * 1000);
            }
        }

        vTaskDelay(10);
    }
}
