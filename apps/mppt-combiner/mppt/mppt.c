#include "mppt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/time.h"

#define MPPT_MAX_PAIRS     32
#define MPPT_KEY_MAX_LEN   16
#define MPPT_VALUE_MAX_LEN 64

MpptData CurrentMpptData = {0};

bool is_charger_data_received = false;

static char keys[MPPT_MAX_PAIRS][MPPT_KEY_MAX_LEN];
static char values[MPPT_MAX_PAIRS][MPPT_VALUE_MAX_LEN];
static int pairCount = 0;

static void trimLine(char* line);
static void addPair(const char* key, const char* value);
static const char* getValue(const char* key);
static void resetFrame(void);
static void commitFrame(void);
static int toIntSafe(const char* value, int fallback);
static float toScaledFloat(const char* value, float scale);
static const char* mapState(const char* cs);
static void copyString(char* dest, size_t destSize, const char* src);

static uint32_t millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}

void processLine(char* line)
{
    if (line == NULL)
    {
        return;
    }

    trimLine(line);

    if (line[0] == '\0')
    {
        return;
    }

    char* tabPos = strchr(line, '\t');

    if (tabPos == NULL)
    {
        return;
    }

    *tabPos = '\0';

    const char* key = line;
    const char* value = tabPos + 1;

    addPair(key, value);

    if (strcmp(key, "Checksum") == 0)
    {
        commitFrame();
        resetFrame();
        is_charger_data_received = true;
    }
}

static void addPair(const char* key, const char* value)
{
    if (pairCount >= MPPT_MAX_PAIRS)
    {
        return;
    }

    copyString(keys[pairCount], sizeof(keys[pairCount]), key);
    copyString(values[pairCount], sizeof(values[pairCount]), value);

    pairCount++;
}

static void commitFrame(void)
{
    const char* fw = getValue("FW");

    CurrentMpptData.deviceInstance = 256;

    copyString(CurrentMpptData.productId, sizeof(CurrentMpptData.productId), getValue("PID"));

    if (strlen(fw) >= 3)
    {
        snprintf(CurrentMpptData.firmwareVersion, sizeof(CurrentMpptData.firmwareVersion), "%c.%.2s", fw[0], fw + 1);
    }
    else
    {
        copyString(CurrentMpptData.firmwareVersion, sizeof(CurrentMpptData.firmwareVersion), fw);
    }

    copyString(CurrentMpptData.serialNumber, sizeof(CurrentMpptData.serialNumber), getValue("SER#"));

    copyString(CurrentMpptData.stateText, sizeof(CurrentMpptData.stateText), mapState(getValue("CS")));

    CurrentMpptData.errorCode = toIntSafe(getValue("ERR"), 0);
    CurrentMpptData.batteryVoltageV = toScaledFloat(getValue("V"), 1000.0f);
    CurrentMpptData.batteryCurrentA = toScaledFloat(getValue("I"), 1000.0f);
    CurrentMpptData.panelVoltageV = toScaledFloat(getValue("VPV"), 1000.0f);
    CurrentMpptData.panelPowerW = toIntSafe(getValue("PPV"), 0);
    CurrentMpptData.yieldTodayKWh = toScaledFloat(getValue("H20"), 100.0f);
    CurrentMpptData.yieldYesterdayKWh = toScaledFloat(getValue("H22"), 100.0f);
    CurrentMpptData.maxPowerTodayW = toIntSafe(getValue("H21"), 0);
    CurrentMpptData.maxPowerYesterdayW = toIntSafe(getValue("H23"), 0);
    CurrentMpptData.loadCurrentA = toScaledFloat(getValue("IL"), 1000.0f);
    CurrentMpptData.loadOutputState = toIntSafe(getValue("LOAD"), 0) == 1;
    CurrentMpptData.chargerModeId = toIntSafe(getValue("MPPT"), 0);
    CurrentMpptData.frameValid = true;
    CurrentMpptData.lastUpdateMs = millis();
}

static const char* getValue(const char* key)
{
    for (int i = 0; i < pairCount; i++)
    {
        if (strcmp(keys[i], key) == 0)
        {
            return values[i];
        }
    }

    return "";
}

static void resetFrame(void)
{
    pairCount = 0;

    memset(keys, 0, sizeof(keys));
    memset(values, 0, sizeof(values));
}

static const char* mapState(const char* cs)
{
    if (strcmp(cs, "0") == 0) { return "Off"; }
    if (strcmp(cs, "2") == 0) { return "Fault"; }
    if (strcmp(cs, "3") == 0) { return "Bulk"; }
    if (strcmp(cs, "4") == 0) { return "Absorption"; }
    if (strcmp(cs, "5") == 0) { return "Float"; }
    if (strcmp(cs, "7") == 0) { return "Equalize"; }

    return cs;
}

static int toIntSafe(const char* value, int fallback)
{
    if (value == NULL || value[0] == '\0')
    {
        return fallback;
    }

    return (int)strtol(value, NULL, 10);
}

static float toScaledFloat(const char* value, float scale)
{
    if (value == NULL || value[0] == '\0' || scale == 0.0f)
    {
        return 0.0f;
    }

    return strtof(value, NULL) / scale;
}

static void trimLine(char* line)
{
    char* start = line;

    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
    {
        start++;
    }

    if (start != line)
    {
        memmove(line, start, strlen(start) + 1);
    }

    size_t len = strlen(line);

    while (len > 0)
    {
        char c = line[len - 1];

        if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
        {
            break;
        }

        line[len - 1] = '\0';
        len--;
    }
}

static void copyString(char* dest, size_t destSize, const char* src)
{
    if (dest == NULL || destSize == 0)
    {
        return;
    }

    if (src == NULL)
    {
        dest[0] = '\0';
        return;
    }

    snprintf(dest, destSize, "%s", src);
}
