#include "test_tof.h"

#include "distance.h"
#include "driver_vl53l0x.h"
#include "hal_i2c.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include "hal_time.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define TOF_READY_TIMEOUT_US 100000U
#define TOF_POLL_INTERVAL_MS 1U
#define TOF_TEST_PERIOD_US 500000U

static bool sensor_ready = false;
static bool test_stopped = false;
static bool first_measurement = true;
static uint32_t last_measurement_us = 0U;

static const char *status_name(vl53l0x_status_t status)
{
    switch (status) {
    case VL53L0X_OK:
        return "OK";
    case VL53L0X_ERROR_INIT:
        return "error de inicializacion";
    case VL53L0X_ERROR_TIMEOUT:
        return "timeout del sensor";
    case VL53L0X_ERROR_INVALID_MEASUREMENT:
        return "medicion invalida";
    case VL53L0X_ERROR_COMMUNICATION:
        return "error de comunicacion";
    case VL53L0X_ERROR_INTERNAL_API:
        return "error interno de la API";
    default:
        return "estado desconocido";
    }
}

static void report_status(const char *operation,
                          vl53l0x_status_t status)
{
    char message[120];
    snprintf(message, sizeof(message), "%s: %s (%d)",
             operation, status_name(status), (int)status);
    telemetry_app_send_line(message);
}

static void report_distance(float raw_cm, float calibrated_cm)
{
    char message[140];
    const uint32_t raw_centi_cm = (uint32_t)(raw_cm * 100.0f + 0.5f);

    if (calibrated_cm >= 0.0f) {
        const uint32_t calibrated_centi_cm =
            (uint32_t)(calibrated_cm * 100.0f + 0.5f);
        snprintf(message, sizeof(message),
                 "Cruda: %lu.%02lu cm | Calibrada: %lu.%02lu cm",
                 (unsigned long)(raw_centi_cm / 100U),
                 (unsigned long)(raw_centi_cm % 100U),
                 (unsigned long)(calibrated_centi_cm / 100U),
                 (unsigned long)(calibrated_centi_cm % 100U));
    } else {
        snprintf(message, sizeof(message),
                 "Cruda: %lu.%02lu cm | Calibrada: fuera del rango de la tabla",
                 (unsigned long)(raw_centi_cm / 100U),
                 (unsigned long)(raw_centi_cm % 100U));
    }
    telemetry_app_send_line(message);
}

static void stop_after_error(const char *operation,
                             vl53l0x_status_t status)
{
    report_status(operation, status);
    telemetry_app_send_line(
        "Test ToF detenido; reiniciar la placa para reintentar.");
    test_stopped = true;
}

void test_tof_init(void)
{
    if (!telemetry_app_init()) {
        hal_telemetry_write("ERROR: no se pudo iniciar la telemetria.");
    }

    telemetry_app_send_line("=== TEST TOF ===");

    if (!hal_time_init()) {
        telemetry_app_send_line("ERROR: no se pudo inicializar el reloj.");
        test_stopped = true;
        return;
    }

    if (!hal_i2c_init(HAL_I2C_BUS_A)) {
        telemetry_app_send_line("ERROR: no se pudo inicializar el bus I2C A.");
        test_stopped = true;
        return;
    }

    vl53l0x_status_t status = vl53l0x_init();
    if (status != VL53L0X_OK) {
        stop_after_error("Inicio VL53L0X", status);
        return;
    }

    status = vl53l0x_set_profile(VL53L0X_PROFILE_HIGH_SPEED);
    if (status != VL53L0X_OK) {
        stop_after_error("Perfil High Speed", status);
        return;
    }

    sensor_ready = true;
    telemetry_app_send_line(
        "VL53L0X listo. Perfil High Speed; muestra distancia cruda y calibrada en cm.");
}

void test_tof_update(void)
{
    telemetry_app_update();

    if (!sensor_ready || test_stopped) {
        return;
    }

    const uint32_t now_us = hal_time_us();
    if (!first_measurement &&
        (uint32_t)(now_us - last_measurement_us) < TOF_TEST_PERIOD_US) {
        return;
    }

    first_measurement = false;
    last_measurement_us = now_us;

    vl53l0x_status_t status = vl53l0x_start();
    if (status != VL53L0X_OK) {
        stop_after_error("Inicio de medicion", status);
        return;
    }

    const uint32_t start_us = hal_time_us();
    for (;;) {
        bool ready = false;
        status = vl53l0x_distance_ready(&ready);
        if (status != VL53L0X_OK) {
            stop_after_error("Consulta de dato listo", status);
            return;
        }

        if (ready) {
            float distance_cm = 0.0f;
            status = vl53l0x_get_distance(&distance_cm);
            if (status == VL53L0X_OK) {
                report_distance(distance_cm, distance_calibrate(distance_cm));
            } else if (status == VL53L0X_ERROR_INVALID_MEASUREMENT) {
                report_status("Lectura", status);
            } else {
                stop_after_error("Lectura", status);
            }
            return;
        }

        if (hal_time_elapsed_us(start_us) >= TOF_READY_TIMEOUT_US) {
            telemetry_app_send_line(
                "Timeout esperando dato; test detenido porque no hay cancelacion de medicion.");
            test_stopped = true;
            return;
        }

        hal_delay(TOF_POLL_INTERVAL_MS);
    }
}
