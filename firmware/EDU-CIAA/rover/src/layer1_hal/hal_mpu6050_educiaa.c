#include "hal_mpu6050.h"
#include "sapi.h"

#include <stddef.h>

static bool initialized = false;

// sAPI utiliza +/-16 g y +/-2000 grados/s; calibra giro durante init.
// Sus getters entregan giro con esa correccion. La calibracion compartida
// estima el offset residual sobre estas mediciones.
bool hal_mpu6050_init(uint8_t address) {
    initialized = false;
    if (address != 0x68U && address != 0x69U) {
        return false;
    }
    initialized = mpu60X0Init((MPU60X0_address_t)address) > 0;
    return initialized;
}

bool hal_mpu6050_read_gyro_z(float *gyro_z_rads) {
    if (!initialized || gyro_z_rads == NULL || !mpu60X0Read()) {
        return false;
    }

    *gyro_z_rads = mpu60X0GetGyroZ_rads();
    return true;
}
