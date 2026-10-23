#include "perception_v0.h"

#include "driver_mpu6050_config.h"
#include "angle.h"
#include "distance.h"
#include "hal_telemetry.h"
#include "hal_time.h"
#include "perception_v0_config.h"

#if defined(PERCEPTION_V0_WIFI_TCP)
#include "hal_telemetry_network.h"
#endif

#include <stdio.h>

/* Ensayo V0: combina angulo y distancia y publica theta_deg,distancia_cm. */
static bool initialized;
static double last_angle_deg;

bool perception_v0_init(void) {
    initialized = false;
    last_angle_deg = 0.0;

    hal_telemetry_write("# Arturito protocolo V0: theta_deg,distancia_cm");

    hal_telemetry_write("# IMU: quedate quieto para calibrar gyro Z...");
    if (!angle_init(MPU6050_I2C_ADDRESS)) {
        hal_telemetry_write("# ERROR: no se pudo iniciar o calibrar la IMU.");
        return false;
    }
    hal_telemetry_write("# IMU OK; gyro Z calibrado.");

    if (!distance_init()) {
        hal_telemetry_write("# WARN: VL53L0X no encontrado. distancia_cm=-1");
    } else {
        hal_telemetry_write("# ToF OK");
    }

#if defined(PERCEPTION_V0_WIFI_TCP)
    hal_telemetry_write("# DEBUG: WIFI COMPILADO");
    if (!hal_telemetry_network_start()) {
        hal_telemetry_write("# WARN: no se inicio Wi-Fi; continua la salida serie.");
    }
#else
    hal_telemetry_write("# DEBUG: WIFI NO COMPILADO");
#endif

    hal_telemetry_write("# Datos CSV:");
    initialized = true;
    return true;
}

void perception_v0_step(void) {
    if (!initialized) {
        return;
    }

    double angle_deg = last_angle_deg;
    if (angle_read_deg(&angle_deg)) {
        last_angle_deg = angle_deg;
    }

    const float distance_cm = distance_get();
    char line[48];
    snprintf(line, sizeof(line), "%.3f,%.4f", angle_deg, distance_cm);
    hal_telemetry_write(line);

#if defined(PERCEPTION_V0_WIFI_TCP)
    char tcp_line[sizeof(line) + 1U];
    snprintf(tcp_line, sizeof(tcp_line), "%s\n", line);
    (void)hal_telemetry_network_send_line(tcp_line);
    hal_telemetry_network_poll();
#endif

    hal_delay(PERCEPTION_V0_SAMPLE_PERIOD_MS);
}
