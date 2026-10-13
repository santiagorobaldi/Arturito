#include "hal_telemetry.h"

#include <stdio.h>

void hal_telemetry_write(const char *line) {
    printf("%s\r\n", line);
}