#include "config.h"
#include "hal_imu.h"
#include "hal_time.h"
#include "driver_mpu6050.h"

#include <Arduino.h>

static float theta_raw_rad = 0.0f;
static uint32_t previous_ms = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("# imu_compare: gyro_z_crudo_rad_s,theta_crudo_rad,theta_capa2_rad");

    if (!hal_imu_init()) {
        Serial.println("# ERROR: IMU no encontrada. Halt.");
        while (true) {
            delay(1000);
        }
    }

    mpu6050_init();
    Serial.println("# Calibrando capa 2 (quedate quieto)...");
    mpu6050_calibrate();
    previous_ms = hal_millis();
    Serial.println("# Calibracion OK. Datos CSV:");
}

void loop() {
    const float gyro_z_raw_rads = hal_imu_get_gyro_z_rads();
    const uint32_t now_ms = hal_millis();
    const float dt = (now_ms - previous_ms) / 1000.0f;
    previous_ms = now_ms;

    // Referencia cruda: integra gyro sin calibracion, deadband ni filtrado.
    theta_raw_rad += gyro_z_raw_rads * dt;
    mpu6050_update_gyro(gyro_z_raw_rads);

    Serial.print(gyro_z_raw_rads, 4);
    Serial.print(',');
    Serial.print(theta_raw_rad, 4);
    Serial.print(',');
    Serial.println(mpu6050_get_theta_rad(), 4);
    delay(SAMPLE_PERIOD_MS);
}
