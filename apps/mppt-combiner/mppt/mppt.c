#include "mppt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"

#include "../helpers/helpers.h"
#include "helpers.h"
#include "pico/time.h"

#define MPPT_MAX_PAIRS 32
#define MPPT_KEY_MAX_LEN 16
#define MPPT_VALUE_MAX_LEN 64

mppt_data current_mppt_data = {0};
QueueHandle_t received_mppt_datas = NULL;

bool is_charger_data_received = false;

static char keys[MPPT_MAX_PAIRS][MPPT_KEY_MAX_LEN];
static char values[MPPT_MAX_PAIRS][MPPT_VALUE_MAX_LEN];
static int pairCount = 0;

static void addPair(const char* key, const char* value);
static const char* getValue(const char* key);
static void resetFrame(void);
static void commitFrame(void);
static int toIntSafe(const char* value, int fallback);
float toScaledFloat(const char* value, float scale);
static const char* mapState(const char* cs);
static void copyString(char* dest, size_t destSize, const char* src);

static uint32_t millis(void) { return to_ms_since_boot(get_absolute_time()); }

void processLine(char* line) {
    if (line == NULL) {
        return;
    }

    strtrim(line);

    if (line[0] == '\0') {
        return;
    }

    char* tabPos = strchr(line, '\t');

    if (tabPos == NULL) {
        return;
    }

    *tabPos = '\0';


    const char* key = line;
    const char* value = tabPos + 1;

    strtrim(key);
    strtrim(value);

    addPair(key, value);
    printf(key);
    if (strcmp(key, "Checksum") == 0) {
        commitFrame();
        resetFrame();
        is_charger_data_received = true;
    }
}

bool try_extract_line(char* buffer, char* extracted_line) {
    // Victron VE.Direct is backwards. Every line *starts* with a \n, not *ends*.
    if (buffer[0] != '\n') {
        goto error;
    }

    if (strlen(buffer) < 4) {
        goto error;
    }

    // Trying to figure out if there is more than one line in the buffer.
    char* next_newline = strchr(buffer + 1, '\n');
    int number_of_bytes_to_copy = strlen(buffer);
    if (next_newline != NULL) {
        number_of_bytes_to_copy = next_newline - buffer;
    }

    // Found a line. Extracting. If it is not the last line, then we also need to append a null terminator.
    strncpy(extracted_line, buffer, number_of_bytes_to_copy);
    if (strlen(extracted_line) > number_of_bytes_to_copy) {
        extracted_line[number_of_bytes_to_copy] = '\0';
    }

    // Condensing the input buffer (until it is no more).
    if (next_newline != NULL) {
        strcpy(buffer, next_newline);
    }
    else {
        *buffer = '\0';
    }

    return true;

error:
    *extracted_line = '\0';
    return false;
}

bool try_process_line(char* line, char* key, char* value) {
    if (line == NULL) {
        goto error;
    }

    char* tab_substring = strchr(line, '\t');

    if (tab_substring == NULL) {
        goto error;
    }

    int tab_position = tab_substring - line;

    strncpy(key, line, tab_position);
    key[tab_position] = '\0';

    strcpy(value, tab_substring + 1);

    strtrim(key);
    strtrim(value);

    return true;
error:
    *key = '\0';
    *value = '\0';
    return false;
}


static void addPair(const char* key, const char* value) {
    if (pairCount >= MPPT_MAX_PAIRS) {
        return;
    }

    copyString(keys[pairCount], sizeof(keys[pairCount]), key);
    copyString(values[pairCount], sizeof(values[pairCount]), value);

    pairCount++;
}

static void commitFrame(void) {
    const char* fw = getValue("FW");

    current_mppt_data.deviceInstance = 256;

    copyString(current_mppt_data.productId, sizeof(current_mppt_data.productId),
               getValue("PID"));

    if (strlen(fw) >= 3) {
        snprintf(current_mppt_data.firmwareVersion,
                 sizeof(current_mppt_data.firmwareVersion), "%c.%.2s", fw[0], fw + 1);
    }
    else {
        copyString(current_mppt_data.firmwareVersion,
                   sizeof(current_mppt_data.firmwareVersion), fw);
    }

    copyString(current_mppt_data.serialNumber, sizeof(current_mppt_data.serialNumber),
               getValue("SER#"));

    copyString(current_mppt_data.stateText, sizeof(current_mppt_data.stateText),
               mapState(getValue("CS")));

    current_mppt_data.errorCode = toIntSafe(getValue("ERR"), 0);
    current_mppt_data.batteryVoltageV = toScaledFloat(getValue("V"), 1000.0f);
    current_mppt_data.batteryCurrentA = toScaledFloat(getValue("I"), 1000.0f);
    current_mppt_data.panelVoltageV = toScaledFloat(getValue("VPV"), 1000.0f);
    current_mppt_data.panelPowerW = toIntSafe(getValue("PPV"), 0);
    current_mppt_data.yieldTodayKWh = toScaledFloat(getValue("H20"), 100.0f);
    current_mppt_data.yieldYesterdayKWh = toScaledFloat(getValue("H22"), 100.0f);
    current_mppt_data.maxPowerTodayW = toIntSafe(getValue("H21"), 0);
    current_mppt_data.maxPowerYesterdayW = toIntSafe(getValue("H23"), 0);
    current_mppt_data.yieldTotalKWh = toScaledFloat(getValue("H19"), 100.0f);
    current_mppt_data.daySequenceNumber = toIntSafe(getValue("HSDS"), 0);
    current_mppt_data.loadCurrentA = toScaledFloat(getValue("IL"), 1000.0f);
    current_mppt_data.loadOutputState = toIntSafe(getValue("LOAD"), 0) == 1;
    current_mppt_data.chargerModeId = toIntSafe(getValue("MPPT"), 0);
    current_mppt_data.frameValid = true;
    current_mppt_data.lastUpdateMs = millis();
}

static const char* getValue(const char* key) {
    for (int i = 0; i < pairCount; i++) {
        if (strcmp(keys[i], key) == 0) {
            return values[i];
        }
    }

    return "";
}

static void resetFrame(void) {
    pairCount = 0;

    memset(keys, 0, sizeof(keys));
    memset(values, 0, sizeof(values));
}

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

float toScaledFloat(const char* value, float scale) {
    if (value == NULL || value[0] == '\0' || scale == 0.0f) {
        return 0.0f;
    }

    return strtof(value, NULL) / scale;
}

static void copyString(char* dest, size_t destSize, const char* src) {
    if (dest == NULL || destSize == 0) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    snprintf(dest, destSize, "%s", src);
}
