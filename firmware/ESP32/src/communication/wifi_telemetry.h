#ifndef WIFI_TELEMETRY_H
#define WIFI_TELEMETRY_H

/* Bloque WiFi del ESP32, compartido por llamadas locales y el puente UART. */
bool wifi_telemetry_start(void);
void wifi_telemetry_poll(void);
bool wifi_telemetry_send_line(const char *line);

#endif
