#include "config.h"
#include "driver_vl53l0x.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("# test vl53l0x_only: distancia_m");

    if (!vl53l0x_init()) {
        Serial.println("# ERROR: VL53L0X no encontrado. Halt.");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("# ToF OK");
}

void loop() {
    float d = vl53l0x_get_distance_m();
    Serial.print("0.0000,");
    Serial.println(d, 4);
    delay(SAMPLE_PERIOD_MS);
}
