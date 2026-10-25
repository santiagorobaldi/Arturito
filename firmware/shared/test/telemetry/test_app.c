#include <Arduino.h>
#include "telemetry_app.h"
#include 

unsigned long last_send = 0;
float simulated_sensor = 0.0f;

void void app_init(void)
{
    Serial.begin(115200);
    telemetry_app_init();
    telemetry_log_info("Sistema de telemetria Layer 3 iniciado.");
}

void void app_update(void) {
    // Mantiene viva la conexión TCP y atiende clientes
    telemetry_app_update();

    // Envío periódico cada 500ms
    if (millis() - last_send > 500) {
        last_send = millis();
        simulated_sensor += 0.5f;

        // Prueba de transmisión Layer 3
        telemetry_log_info("Enviando medicion...");
        telemetry_plot_value("distancia_cm", simulated_sensor);
    }
}
