#include "config.h"
#include "driver_mpu6050.h"
#include "odometry.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("# test imu_only: theta_rad");

    if (!mpu6050_init(IMU_I2C_ADDR)) {
        Serial.println("# ERROR: IMU no encontrada. Halt.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("# Calibrando IMU (quedate quieto)...");
    delay(2000);
    if (!mpu6050_calibrate_gyro(1000U, 2U) ||
        !odometry_init(0.0f, 0.017453292519943295f)) {
        Serial.println("# ERROR: fallo calibracion/inicio de orientacion.");
        while (true) { delay(1000); }
    }
    Serial.println("# Calibracion OK");
}

void loop() {
    mpu6050_sample_t sample;
    if (!mpu6050_read(&sample)) {
        odometry_restart_tracking();
        Serial.println("# ERROR: lectura IMU invalida.");
        delay(SAMPLE_PERIOD_MS);
        return;
    }
    if (!odometry_update(sample.gyro_z_rads, millis())) {
        odometry_restart_tracking();
    }
    Serial.print(odometry_get_angle(), 4);
    Serial.println(",-1.0000");
    delay(SAMPLE_PERIOD_MS);
}
