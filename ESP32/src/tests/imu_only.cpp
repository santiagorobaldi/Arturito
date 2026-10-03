#include "config.h"
#include "hal_imu.h"
#include "driver_mpu6050.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("# test imu_only: theta_rad");

    if (!hal_imu_init()) {
        Serial.println("# ERROR: IMU no encontrada. Halt.");
        while (true) {
            delay(1000);
        }
    }

    mpu6050_init();
    Serial.println("# Calibrando IMU (quedate quieto)...");
    mpu6050_calibrate();
    Serial.println("# Calibracion OK");
}

void loop() {
    mpu6050_update();
    Serial.print(mpu6050_get_theta_rad(), 4);
    Serial.println(",-1.0000");
    delay(SAMPLE_PERIOD_MS);
}
