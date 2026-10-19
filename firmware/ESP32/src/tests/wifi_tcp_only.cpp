#include "hal_telemetry_network.h"

#include <Arduino.h>

#include <stdio.h>

namespace {
constexpr uint32_t kPublishIntervalMs = 1000U;
constexpr uint32_t kDiagnosticIntervalMs = 5000U;
uint32_t lastPublishMs = 0U;
uint32_t lastDiagnosticMs = 0U;
uint32_t counter = 0U;
}

void setup() {
    Serial.begin(115200);
    Serial.println("[wifi_tcp_only] ensayo COUNT por Wi-Fi/TCP; puerto serie para diagnóstico.");
    // Si faltan credenciales, la HAL lo informa y el test sigue vivo sin red.
    if (!hal_telemetry_network_start()) {
        Serial.println("[wifi_tcp_only] configure include/wifi_secrets.h y vuelva a compilar.");
    }
}

void loop() {
    // poll() tiene que llamarse seguido: gestiona la conexión mientras el
    // contador y los diagnósticos siguen funcionando.
    hal_telemetry_network_poll();

    const uint32_t now = millis();
    // La resta también funciona cuando millis() vuelve a cero por desborde.
    if (static_cast<uint32_t>(now - lastPublishMs) >= kPublishIntervalMs) {
        lastPublishMs = now;
        char line[32];
        // El salto de línea es parte del protocolo TCP, no solo formato visual.
        snprintf(line, sizeof(line), "COUNT,%lu\n", static_cast<unsigned long>(counter++));
        if (!hal_telemetry_network_send_line(line) &&
            static_cast<uint32_t>(now - lastDiagnosticMs) >= kDiagnosticIntervalMs) {
            Serial.println("[wifi_tcp_only] trama descartada (cliente ausente o envío fallido).");
            lastDiagnosticMs = now;
        }
    }
    // Cede tiempo a Arduino/FreeRTOS sin agregar una tarea propia al ensayo.
    delay(1);
}
