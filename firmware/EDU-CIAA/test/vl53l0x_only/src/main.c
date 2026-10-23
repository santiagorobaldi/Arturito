#include "sapi.h"
#include "hal_i2c.h"
#include "driver_vl53l0x.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

// Cambiar esta constante y recompilar para probar otro perfil.
static const hal_vl53l0x_profile_t TEST_PROFILE = HAL_VL53L0X_PROFILE_DEFAULT;
static const uint32_t POLL_INTERVAL_MS = 5;

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

static void test_sensor(void *context)
{
    (void)context;
    unsigned long sample_number = 0;

    printf("# Test VL53L0X: driver, single y polling del host.\r\n");
    hal_vl53l0x_status_t status = vl53l0x_init();
    printf("# INIT: %s\r\n", status_name(status));
    if (status != HAL_VL53L0X_OK) {
        vTaskDelete(NULL);
        return;
    }

    status = vl53l0x_set_profile(TEST_PROFILE);
    printf("# PERFIL %u: %s (0=DEFAULT, 1=LONG_RANGE, 2=HIGH_SPEED, 3=HIGH_ACCURACY)\r\n",
           (unsigned)TEST_PROFILE, status_name(status));
    if (status != HAL_VL53L0X_OK) {
        vTaskDelete(NULL);
        return;
    }

    for (;;) {
        uint32_t start_ms = tickRead();
        status = vl53l0x_start();
        if (status != HAL_VL53L0X_OK) {
            printf("# START: %s. Prueba detenida.\r\n", status_name(status));
            break;
        }

        bool ready = false;
        // Dormir entre consultas permite ejecutar otras tareas; sin plazo externo.
        while (!ready) {
            status = vl53l0x_distance_ready(&ready);
            if (status != HAL_VL53L0X_OK) {
                break;
            }
            if (!ready) {
                vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
            }
        }
        if (status != HAL_VL53L0X_OK) {
            printf("# READY: %s. Prueba detenida.\r\n", status_name(status));
            break;
        }

        float distance_m;
        status = vl53l0x_get_distance(&distance_m);
        ++sample_number;
        if (status == HAL_VL53L0X_OK) {
            printf("MUESTRA %lu: OK, distancia=%.3f m, tiempo=%lu ms\r\n",
                   sample_number, (double)distance_m,
                   (unsigned long)(uint32_t)(tickRead() - start_ms));
        } else {
            printf("MUESTRA %lu: %s\r\n", sample_number, status_name(status));
            // La muestra invalida ya fue recogida y reconocida por HAL.
            if (status != HAL_VL53L0X_ERROR_INVALID_MEASUREMENT) {
                break;
            }
        }
    }

    vTaskDelete(NULL);
}

int main(void)
{
    boardConfig();
    if (!hal_i2c_init(HAL_I2C_BUS_A)) {
        printf("# ERROR: no se pudo inicializar I2C A.\r\n");
        for (;;) {}
    }

    // La inicializacion del sensor ocurre dentro de esta unica tarea.
    if (xTaskCreate(test_sensor, "vl53l0x", 1024, NULL, 1, NULL) != pdPASS) {
        printf("# ERROR: no se pudo crear la tarea del test.\r\n");
        for (;;) {}
    }
    vTaskStartScheduler();
    printf("# ERROR: FreeRTOS no pudo iniciar.\r\n");
    for (;;) {}
}
