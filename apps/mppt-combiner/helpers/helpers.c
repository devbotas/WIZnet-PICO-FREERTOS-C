#include "helpers.h"

#include <string.h>

void strtrim(char* string) {
    char* start = string;

    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
        start++;
    }

    if (start != string) {
        memmove(string, start, strlen(start) + 1);
    }

    size_t len = strlen(string);

    while (len > 0) {
        char c = string[len - 1];

        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }

        string[len - 1] = '\0';
        len--;
    }
}
