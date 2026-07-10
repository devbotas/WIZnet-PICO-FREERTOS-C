#pragma once

#include <stdint.h>
#include "wizchip_conf.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define DHCP_RETRY_COUNT 5
#define SOCKET_DHCP 0
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)


extern int g_dhcp_get_ip_flag; // <-- Consider converting this to semaphore
extern xSemaphoreHandle dns_semaphore;
extern uint8_t ethernet_buffer[ETHERNET_BUF_MAX_SIZE];
extern wiz_NetInfo network_information;
extern volatile uint32_t g_msec_cnt;

void network_init(void);

void initialize_dhcp(void);
void assign_ip_from_dhcp(void);
void resolve_dhcp_conflict(void);
void repeating_timer_callback(void);

void get_ipi_from_dhcp_task(void* argument);
