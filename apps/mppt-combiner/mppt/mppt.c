#include "mppt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"

#include "../helpers/helpers.h"
#include "../rest_client/rest_client.h"
#include "pico/time.h"

#define MPPT_MAX_PAIRS 32
#define MPPT_KEY_MAX_LEN 16
#define MPPT_VALUE_MAX_LEN 64

static int toIntSafe(const char* value, int fallback);
float to_scaled_float(const char* value, float scale);
static const char* mapState(const char* cs);

static uint32_t millis(void) { return to_ms_since_boot(get_absolute_time()); }

static const char* mapState(const char* cs) {
    if (strcmp(cs, "0") == 0) {
        return "Off";
    }
    if (strcmp(cs, "2") == 0) {
        return "Fault";
    }
    if (strcmp(cs, "3") == 0) {
        return "Bulk";
    }
    if (strcmp(cs, "4") == 0) {
        return "Absorption";
    }
    if (strcmp(cs, "5") == 0) {
        return "Float";
    }
    if (strcmp(cs, "7") == 0) {
        return "Equalize";
    }

    return cs;
}

static int toIntSafe(const char* value, int fallback) {
    if (value == NULL || value[0] == '\0') {
        return fallback;
    }

    return (int)strtol(value, NULL, 10);
}

void post_rest_continuously_task(void* argument) {
    const QueueHandle_t queue = argument;
    mppt_data_t data_to_send;
    const uint32_t server_ip = (172 << 24) | (16 << 16) | (7 << 8) | 1;
    const uint16_t port = 8080;
    static char body[512];

    while (1) {
        if (xQueueReceive(queue, &data_to_send, portMAX_DELAY) != pdPASS) { continue; }

        snprintf(body, sizeof(body),
                 "{"
                 "\"product_id\":\"%s\","
                 "\"firmware_version\":\"%s\","
                 "\"serial_number\":\"%s\","
                 "\"device_instance\":%d,"
                 "\"state_text\":\"%s\","
                 "\"battery_voltage_v\":%.2f,"
                 "\"battery_current_a\":%.2f,"
                 "\"panel_voltage_v\":%.2f,"
                 "\"panel_power_w\":%d,"
                 "\"yield_today_kwh\":%.3f,"
                 "\"load_current_a\":%.2f,"
                 "\"load_output_state\":%s"
                 "}",
                 data_to_send.product_id,
                 data_to_send.firmware_version,
                 data_to_send.serial_number,
                 data_to_send.device_instance,
                 data_to_send.state_text,
                 data_to_send.battery_voltage_v,
                 data_to_send.battery_current_a,
                 data_to_send.panel_voltage_v,
                 data_to_send.panel_power_w,
                 data_to_send.yield_today_kwh,
                 data_to_send.load_current_a,
                 data_to_send.load_output_state ? "true" : "false");

        rest_post(server_ip, port, "api", body, NULL, 0);
    }
}
