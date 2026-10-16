#include "driver_mpu6050.h"
#include "hal_mpu6050.h"
#include "hal_time.h"

#include <stddef.h>

static bool initialized;
static float offset_x, offset_y, offset_z;

bool mpu6050_init(uint8_t address) {
    initialized = false;
    if ((address != 0x68U && address != 0x69U) || !hal_mpu6050_init(address)) {
        return false;
    }
    offset_x = offset_y = offset_z = 0.0f;
    initialized = true;
    return true;
}

bool mpu6050_read(mpu6050_sample_t *sample) {
    mpu6050_sample_t next;
    if (!initialized || sample == NULL || !hal_mpu6050_read(&next)) {
        return false;
    }
    next.gyro_x_rads -= offset_x;
    next.gyro_y_rads -= offset_y;
    next.gyro_z_rads -= offset_z;
    *sample = next;
    return true;
}

bool mpu6050_calibrate_gyro(uint32_t sample_count, uint32_t interval_ms) {
    double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
    if (!initialized || sample_count == 0U || interval_ms == 0U) {
        return false;
    }
    mpu6050_sample_t sample;
    for (uint32_t i = 0; i < sample_count; ++i) {
        if (!hal_mpu6050_read(&sample)) {
            return false;
        }
        sum_x += sample.gyro_x_rads;
        sum_y += sample.gyro_y_rads;
        sum_z += sample.gyro_z_rads;
        if (i + 1U < sample_count) {
            hal_delay(interval_ms);
        }
    }
    offset_x = (float)(sum_x / sample_count);
    offset_y = (float)(sum_y / sample_count);
    offset_z = (float)(sum_z / sample_count);
    return true;
}
