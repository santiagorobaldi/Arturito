#include "driver_mpu6050.h"
#include "hal_mpu6050.h"

#include <stddef.h>

/* El driver valida la inicializacion y reenvia gyro Z; la aplicacion corrige e integra. */
static bool initialized;

bool mpu6050_init(uint8_t address) {
    initialized = false;
    if ((address != 0x68U && address != 0x69U) ||
        !hal_mpu6050_init(address)) {
        return false;
    }

    initialized = true;
    return true;
}

bool mpu6050_read_gyro_z(float *gyro_z_rads) {
    if (!initialized || gyro_z_rads == NULL) {
        return false;
    }

    return hal_mpu6050_read_gyro_z(gyro_z_rads);
}
