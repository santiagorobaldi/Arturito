#include "perception_v0.h"

#include "driver_mpu6050.h"
#include "driver_mpu6050_config.h"
#include "driver_vl53l0x.h"
#include "hal_telemetry.h"
#include "hal_time.h"
#include "perception_v0_config.h"

#if defined(PERCEPTION_V0_WIFI_TCP)
#include "hal_telemetry_network.h"
#endif

#include <stdio.h>

static bool initialized;
static double last_angle_deg;

bool perception_v0_init(void) {
    initialized = false;
    last_angle_deg = 0.0;

    hal_telemetry_write("# Arturito protocolo V0: theta_deg,distancia_cm");

    if (!mpu6050_init(MPU6050_I2C_ADDRESS)) {
        hal_telemetry_write("# ERROR: IMU MPU6050 no encontrada.");
        return false;
    }
    hal_telemetry_write("# IMU OK");

    if (!vl53l0x_init()) {
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

    hal_telemetry_write("# Calibrando IMU (quedate quieto)...");
    hal_delay(2000U);
    if (!mpu6050_calibrate_gyro_z(MPU6050_CALIBRATION_SAMPLE_COUNT,
                                  MPU6050_CALIBRATION_INTERVAL_MS)) {
        hal_telemetry_write("# ERROR: fallo la calibracion del gyro Z.");
        return false;
    }

    hal_telemetry_write("# Calibracion OK. Datos CSV:");
    initialized = true;
    return true;
}

void perception_v0_step(void) {
    if (!initialized) {
        return;
    }

    double angle_deg = last_angle_deg;
    (void)mpu6050_read_angle_deg(&angle_deg);
    last_angle_deg = angle_deg;

    /* La API del driver devuelve distancia calibrada o -1 si no es valida. */
    const float distance_cm = vl53l0x_get_distance_cm();
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
