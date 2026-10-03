#include "config.h"
#include "hal_tof.h"
#include "driver_distancia.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("# test tof_only: distancia_m");

    if (!hal_tof_init()) {
        Serial.println("# ERROR: VL53L0X no encontrado. Halt.");
        while (true) {
            delay(1000);
        }
    }
    distancia_init();
    Serial.println("# ToF OK");
}

void loop() {
    float d = distancia_get_m();
    Serial.print("0.0000,");
    Serial.println(d, 4);
    delay(SAMPLE_PERIOD_MS);
}
