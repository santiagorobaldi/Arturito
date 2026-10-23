#include "perception_v0.h"
#include "hal_telemetry.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    if (!perception_v0_init()) {
        hal_telemetry_write("# ERROR: no se pudo iniciar la aplicacion.");
    }
}

void loop() {
    perception_v0_step();
}
