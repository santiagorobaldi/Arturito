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

bool hal_mpu6050_read(mpu6050_sample_t *sample) {
    if (!initialized || sample == NULL || !mpu60X0Read()) {
        return false;
    }

    mpu6050_sample_t next;
    next.accel_x_mss = mpu60X0GetAccelX_mss();
    next.accel_y_mss = mpu60X0GetAccelY_mss();
    next.accel_z_mss = mpu60X0GetAccelZ_mss();
    next.gyro_x_rads = mpu60X0GetGyroX_rads();
    next.gyro_y_rads = mpu60X0GetGyroY_rads();
    next.gyro_z_rads = mpu60X0GetGyroZ_rads();
    *sample = next;
    return true;
}
