#include "hal_telemetry.h"
#include "communication/wifi_telemetry.h"
#include <Arduino.h>

bool hal_telemetry_init(void)
{
    return wifi_telemetry_start();
}

void hal_telemetry_update(void)
{
    wifi_telemetry_poll();
}

bool hal_telemetry_send_line(const char *line)
{
    return wifi_telemetry_send_line(line);
}

void hal_telemetry_write(const char *line)
{
    Serial.println(line);
}
