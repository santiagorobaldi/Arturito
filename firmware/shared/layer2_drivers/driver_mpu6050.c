#include "driver_mpu6050.h"
#include "driver_mpu6050_config.h"
#include "hal_mpu6050.h"
#include "hal_time.h"

#include <math.h>
#include <stddef.h>

static bool initialized;
static float offset_z;
static double angle_rad;
static float previous_velocity_rads;
static uint32_t previous_time_us;
static bool has_previous;
static const double RAD_TO_DEG = 57.2957795130823208768;

static bool read_corrected_gyro_z(float *gyro_z_rads) {
    float next;
    if (!hal_mpu6050_read_gyro_z(&next) || !isfinite(next)) {
        return false;
    }
    *gyro_z_rads = next - offset_z;
    return isfinite(*gyro_z_rads);
}

bool mpu6050_init(uint8_t address) {
    initialized = false;
    if ((address != 0x68U && address != 0x69U) || !hal_mpu6050_init(address)) {
        return false;
    }
    offset_z = 0.0f;
    angle_rad = 0.0;
    previous_velocity_rads = 0.0f;
    previous_time_us = 0U;
    has_previous = false;
    initialized = true;
    return true;
}

bool mpu6050_read_gyro_z(float *gyro_z_rads) {
    if (!initialized || gyro_z_rads == NULL ||
        !read_corrected_gyro_z(gyro_z_rads)) {
        return false;
    }
    return true;
}

bool mpu6050_read_angle_deg(double *angle_deg) {
    if (!initialized || angle_deg == NULL) {
        return false;
    }

    *angle_deg = angle_rad * RAD_TO_DEG;

    float angular_velocity_rads;
    if (!read_corrected_gyro_z(&angular_velocity_rads)) {
        has_previous = false;
        return false;
    }

    const uint32_t sample_time_us = hal_micros();
    if (has_previous) {
        const uint32_t elapsed_us = sample_time_us - previous_time_us;
        if (elapsed_us == 0U || elapsed_us > MPU6050_MAX_SAMPLE_GAP_US) {
            has_previous = false;
            return false;
        }
        const double dt_s = (double)elapsed_us / 1000000.0;
        const double mean_velocity =
            ((double)previous_velocity_rads + angular_velocity_rads) * 0.5;
        angle_rad += mean_velocity * dt_s;
    }

    previous_velocity_rads = angular_velocity_rads;
    previous_time_us = sample_time_us;
    has_previous = true;
    *angle_deg = angle_rad * RAD_TO_DEG;
    return true;
}

bool mpu6050_calibrate_gyro_z(uint32_t sample_count, uint32_t interval_ms) {
    double sum_z = 0.0;
    if (!initialized || sample_count == 0U || interval_ms == 0U) {
        return false;
    }
    has_previous = false;
    float gyro_z_rads;
    for (uint32_t i = 0; i < sample_count; ++i) {
        if (!hal_mpu6050_read_gyro_z(&gyro_z_rads) || !isfinite(gyro_z_rads)) {
            return false;
        }
        sum_z += gyro_z_rads;
        if (i + 1U < sample_count) {
            hal_delay(interval_ms);
        }
    }
    offset_z = (float)(sum_z / sample_count);
    return true;
}
