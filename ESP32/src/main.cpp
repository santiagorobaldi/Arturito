#include "app_runtime.h"
#include "hal_telemetry.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    if (!app_runtime_start()) {
        hal_telemetry_write("# ERROR: no se pudo iniciar la aplicacion.");
        for (;;) {
            delay(1000);
        }
    }
}

void loop() {
}
