#include "hal_mpu6050.h"
#include "sapi.h"

#include <stddef.h>

static bool initialized = false;

bool hal_mpu6050_init(uint8_t address) {
    initialized = false;
    if (address != 0x68U && address != 0x69U) {
        return false;
    }

    initialized = mpu60X0Init((MPU60X0_address_t)address) > 0;
    return initialized;
}

bool hal_mpu6050_read(hal_mpu6050_sample_t *sample) {
    if (!initialized || sample == NULL || !mpu60X0Read()) {
        return false;
    }

    hal_mpu6050_sample_t next;
    next.accel_x_mss = mpu60X0GetAccelX_mss();
    next.accel_y_mss = mpu60X0GetAccelY_mss();
    next.accel_z_mss = mpu60X0GetAccelZ_mss();
    next.gyro_x_rads = mpu60X0GetGyroX_rads();
    next.gyro_y_rads = mpu60X0GetGyroY_rads();
    next.gyro_z_rads = mpu60X0GetGyroZ_rads();
    *sample = next;
    return true;
}

bool hal_mpu6050_read_gyro_z(float *gyro_z_rads) {
    if (gyro_z_rads == NULL) {
        return false;
    }

    hal_mpu6050_sample_t sample;
    if (!hal_mpu6050_read(&sample)) {
        return false;
    }

    *gyro_z_rads = sample.gyro_z_rads;
    return true;
}
