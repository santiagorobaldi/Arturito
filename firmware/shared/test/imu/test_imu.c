#include "main.h"
#include "angle.h"
#include "driver_mpu6050_config.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include "hal_time.h"

#include <stdio.h>

static bool ready = false;
static uint32_t last_sample_ms;
static uint32_t last_send_ms;

void app_init(void)
{
    if (!telemetry_app_init()) {
        hal_telemetry_write("ERROR: no se pudo iniciar la telemetria.");
        return;
    }

    telemetry_app_send_line("TEST IMU: mantener quieto mientras se calibra.");
    if (!angle_init(MPU6050_I2C_ADDRESS)) {
        telemetry_app_send_line("ERROR: no se pudo inicializar o calibrar la IMU.");
        return;
    }

    last_sample_ms = hal_millis();
    last_send_ms = last_sample_ms;
    ready = true;
    telemetry_app_send_line("IMU lista. Enviando angulo Z por telemetria; ya se puede girar.");
}

void app_update(void)
{
    telemetry_app_update();
    if (!ready) {
        return;
    }

    const uint32_t now_ms = hal_millis();
    if ((uint32_t)(now_ms - last_sample_ms) < 10U) {
        return;
    }
    last_sample_ms = now_ms;

    double angle_deg;
    const bool valid = angle_read_deg(&angle_deg);
    if ((uint32_t)(now_ms - last_send_ms) < 100U) {
        return;
    }
    last_send_ms = now_ms;

    char line[64];
    if (valid) {
        snprintf(line, sizeof(line), "Angulo Z: %.2f grados", angle_deg);
    } else {
        snprintf(line, sizeof(line), "ERROR: no se pudo leer el angulo.");
    }
    telemetry_app_send_line(line);
}
