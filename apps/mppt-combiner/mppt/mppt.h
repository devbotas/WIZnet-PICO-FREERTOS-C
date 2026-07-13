#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MPPT_PRODUCT_ID_MAX_LEN       32
#define MPPT_FIRMWARE_VERSION_MAX_LEN 32
#define MPPT_SERIAL_NUMBER_MAX_LEN    32
#define MPPT_STATE_TEXT_MAX_LEN       32

typedef struct
{
    char productId[MPPT_PRODUCT_ID_MAX_LEN];
    char firmwareVersion[MPPT_FIRMWARE_VERSION_MAX_LEN];
    char serialNumber[MPPT_SERIAL_NUMBER_MAX_LEN];
    char stateText[MPPT_STATE_TEXT_MAX_LEN];

    int deviceInstance;
    int errorCode;
    float batteryVoltageV;
    float batteryCurrentA;
    float panelVoltageV;
    int panelPowerW;
    float yieldTodayKWh;
    float yieldYesterdayKWh;
    int maxPowerTodayW;
    int maxPowerYesterdayW;
    float loadCurrentA;
    bool loadOutputState;
    int chargerModeId;
    bool frameValid;
    uint32_t lastUpdateMs;
} MpptData;

extern MpptData CurrentMpptData;
extern bool is_charger_data_received;

void processLine(char* line);
