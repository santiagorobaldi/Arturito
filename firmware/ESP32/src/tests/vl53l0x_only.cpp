#include "driver_vl53l0x.h"

#include <Arduino.h>

// Cambiar esta constante y recompilar para probar otro perfil.
static const hal_vl53l0x_profile_t TEST_PROFILE = HAL_VL53L0X_PROFILE_DEFAULT;
static const uint32_t POLL_INTERVAL_MS = 5;
static bool test_running = false;
static unsigned long sample_number = 0;

static const char *status_name(hal_vl53l0x_status_t status)
{
    switch (status) {
    case HAL_VL53L0X_OK: return "OK";
    case HAL_VL53L0X_ERROR_INIT: return "ERROR_INIT";
    case HAL_VL53L0X_ERROR_TIMEOUT: return "ERROR_TIMEOUT";
    case HAL_VL53L0X_ERROR_INVALID_MEASUREMENT: return "ERROR_INVALID_MEASUREMENT";
    case HAL_VL53L0X_ERROR_COMMUNICATION: return "ERROR_COMMUNICATION";
    default: return "ERROR_DESCONOCIDO";
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("# Test VL53L0X: driver, single y polling del host.");

    hal_vl53l0x_status_t status = vl53l0x_init();
    Serial.printf("# INIT: %s\r\n", status_name(status));
    if (status != HAL_VL53L0X_OK) {
        return;
    }

    status = vl53l0x_set_profile(TEST_PROFILE);
    Serial.printf("# PERFIL %u: %s (0=DEFAULT, 1=LONG_RANGE, 2=HIGH_SPEED, 3=HIGH_ACCURACY)\r\n",
                  (unsigned)TEST_PROFILE, status_name(status));
    test_running = (status == HAL_VL53L0X_OK);
}

void loop()
{
    if (!test_running) {
        delay(1000);
        return;
    }

    unsigned long start_ms = millis();
    hal_vl53l0x_status_t status = vl53l0x_start();
    if (status != HAL_VL53L0X_OK) {
        Serial.printf("# START: %s. Prueba detenida.\r\n", status_name(status));
        test_running = false;
        return;
    }

    bool ready = false;
    // Una consulta por iteracion. delay cede ejecucion; no hay timeout del test.
    while (!ready) {
        status = vl53l0x_distance_ready(&ready);
        if (status != HAL_VL53L0X_OK) {
            Serial.printf("# READY: %s. Prueba detenida.\r\n", status_name(status));
            test_running = false;
            return;
        }
        if (!ready) {
            delay(POLL_INTERVAL_MS);
        }
    }

    float distance_m;
    status = vl53l0x_get_distance(&distance_m);
    ++sample_number;
    if (status == HAL_VL53L0X_OK) {
        Serial.printf("MUESTRA %lu: OK, distancia=%.3f m, tiempo=%lu ms\r\n",
                      sample_number, (double)distance_m, millis() - start_ms);
    } else {
        Serial.printf("MUESTRA %lu: %s\r\n", sample_number, status_name(status));
        // Una muestra invalida ya recogida permite comenzar otra medicion.
        if (status != HAL_VL53L0X_ERROR_INVALID_MEASUREMENT) {
            test_running = false;
        }
    }
}
