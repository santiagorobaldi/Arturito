#include "hal_telemetry.h"

#include <Arduino.h>

void hal_telemetry_write(const char *line) {
    Serial.println(line);
}