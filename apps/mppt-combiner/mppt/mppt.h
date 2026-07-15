#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"

#define MPPT_PRODUCT_ID_MAX_LEN       32
#define MPPT_FIRMWARE_VERSION_MAX_LEN 32
#define MPPT_SERIAL_NUMBER_MAX_LEN    32
#define MPPT_STATE_TEXT_MAX_LEN       32

typedef struct {
    char product_id[MPPT_PRODUCT_ID_MAX_LEN];
    char firmware_version[MPPT_FIRMWARE_VERSION_MAX_LEN];
    char serial_number[MPPT_SERIAL_NUMBER_MAX_LEN];
    char state_text[MPPT_STATE_TEXT_MAX_LEN];

    int device_instance;
    int error_code;
    float battery_voltage_v;
    float battery_current_a;
    float panel_voltage_v;
    int panel_power_w;
    float yield_today_kwh;
    float yield_yesterday_kwh;
    int max_power_today_w;
    int max_power_yesterday_w;
    float yield_total_kwh;
    int day_sequence_number;
    float load_current_a;
    bool load_output_state;
    int charger_mode_id;
    bool frame_valid;
    uint32_t last_update_ms;
} mppt_data_t;

extern mppt_data_t current_mppt_data;
extern bool is_charger_data_received;
void post_rest_continuously_task(void* argument);
