#include "serial-pio.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"

serialpio_rx_buffer_t rxbuf = {0};

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

float to_scaled_float(const char* value, float scale) {
    if (value == NULL || value[0] == '\0' || scale == 0.0f) {
        return 0.0f;
    }

    return strtof(value, NULL) / scale;
}
