#include "wifi_telemetry.h"
#include <Arduino.h>
#include <WiFi.h>
#include <string.h>

static WiFiServer server(3333);
static WiFiClient client;

bool wifi_telemetry_start(void)
{
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP("Arturito", "arturito123")) {
        Serial.println("ERROR: no se pudo crear la red Arturito.");
        return false;
    }
    server.begin();
    Serial.print("WiFi Arturito | IP: ");
    Serial.print(WiFi.softAPIP());
    Serial.println(" | TCP: 3333");
    return true;
}

void wifi_telemetry_poll(void)
{
    if (!client.connected()) {
        client.stop();
        client = server.available();
        if (client) {
            Serial.println("Cliente TCP conectado.");
        }
    }
}

bool wifi_telemetry_send_line(const char *line)
{
    if (line == nullptr || !client.connected()) {
        return false;
    }
    const size_t length = strlen(line);
    if (client.write((const uint8_t *)line, length) != length ||
        client.write('\n') != 1U) {
        client.stop();
        return false;
    }
    return true;
}
