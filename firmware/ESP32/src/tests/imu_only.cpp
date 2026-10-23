#include "driver_mpu6050.h"
#include "driver_mpu6050_config.h"

#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("# IMU MPU6050: gyro_z_dps");

    if (!mpu6050_init(MPU6050_I2C_ADDRESS)) {
        Serial.println("# ERROR: no se pudo inicializar la IMU.");
        while (true) {
            delay(1000U);
        }
    }

    Serial.println("# IMU OK");
}

void loop() {
    float gyro_z_rads;
    if (mpu6050_read_gyro_z(&gyro_z_rads)) {
        const float gyro_z_dps = gyro_z_rads * 57.2957795f;
        Serial.printf("%.3f\n", gyro_z_dps);
    }
    delay(10U);
}
