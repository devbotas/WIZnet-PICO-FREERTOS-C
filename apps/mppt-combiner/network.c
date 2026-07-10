#include "network.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include <task.h>
#include <stdio.h>
#include "dhcp.h"

int g_dhcp_get_ip_flag = 0; // <-- Consider converting this to semaphore
xSemaphoreHandle dns_semaphore = NULL;

uint8_t ethernet_buffer[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

wiz_NetInfo network_information =
{
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip = {192, 168, 11, 2},
    .sn = {255, 255, 255, 0},
    .gw = {192, 168, 11, 1},
    .dns = {8, 8, 8, 8},
    .dhcp = NETINFO_DHCP
};

void initialize_dhcp(void)
{
    printf(" DHCP client running\n");

    DHCP_init(SOCKET_DHCP, ethernet_buffer);

    reg_dhcp_cbfunc(assign_ip_from_dhcp, assign_ip_from_dhcp, resolve_dhcp_conflict);

    g_dhcp_get_ip_flag = 0;
}

void assign_ip_from_dhcp(void)
{
    getIPfromDHCP(network_information.ip);
    getGWfromDHCP(network_information.gw);
    getSNfromDHCP(network_information.sn);
    getDNSfromDHCP(network_information.dns);

    network_information.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(network_information); // apply from DHCP

    print_network_information(network_information);
    printf(" DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

void resolve_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\n");

    // halt or reset or any...
    while (1)
    {
        vTaskDelay(1000 * 1000);
    }
}




