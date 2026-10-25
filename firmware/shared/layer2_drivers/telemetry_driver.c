#include "telemetry_driver.h"
#include "hal_telemetry_network.h"
#include <stdio.h>

bool telemetry_driver_init(void) {
    return hal_telemetry_network_start();
}

void telemetry_driver_poll(void) {
    hal_telemetry_network_poll();
}

bool telemetry_send_text(telemetry_log_level_t level, const char *message) {
    if (message == NULL) return false;

    const char *tag = "INFO";
    if (level == TELEMETRY_LOG_WARN) tag = "WARN";
    else if (level == TELEMETRY_LOG_ERROR) tag = "ERROR";

    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "[%s] %s\n", tag, message);
    if (len < 0 || len >= (int)sizeof(buffer)) return false;

    return hal_telemetry_network_send_line(buffer);
}

bool telemetry_send_variable_float(const char *key, float value) {
    if (key == NULL) return false;

    char buffer[128];
    // Formato apto para parsear en Processing: "DATA:temp=23.50\n"
    int len = snprintf(buffer, sizeof(buffer), "DATA:%s=%.2f\n", key, value);
    if (len < 0 || len >= (int)sizeof(buffer)) return false;

    return hal_telemetry_network_send_line(buffer);
}