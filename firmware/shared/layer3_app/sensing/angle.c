#include "angle.h"

#include "driver_mpu6050.h"
#include "hal_time.h"

#include <math.h>
#include <stddef.h>

/* Estima bias y ruido de gyro Z en reposo e integra con dt medido entre lecturas. */
static const uint32_t kSettleMs = 2000U;
static const uint32_t kCalibrationSamples = 500U;
static const uint32_t kCalibrationIntervalMs = 10U;
static const uint32_t kMaxSampleGapUs = 100000U;
static const double kRadiansToDegrees = 57.29577951308232;

static bool initialized;
static bool has_previous_sample;
static float gyro_z_offset_rads;
static float gyro_z_stddev_rads;
static float previous_gyro_z_rads;
static uint32_t previous_sample_time_us;
static double angle_rad;

static bool calibrate_gyro_z(void) {
    double mean = 0.0;
    double squared_deviations = 0.0;
    float sample;

    hal_delay(kSettleMs);
    for (uint32_t i = 0U; i < kCalibrationSamples; ++i) {
        if (!mpu6050_read_gyro_z(&sample) || !isfinite(sample)) {
            return false;
        }
        // Actualiza promedio y dispersion sin almacenar las muestras (Welford).
        const double delta = (double)sample - mean;
        mean += delta / (double)(i + 1U);
        squared_deviations += delta * ((double)sample - mean);
        if (i + 1U < kCalibrationSamples) {
            hal_delay(kCalibrationIntervalMs);
        }
    }

    gyro_z_offset_rads = (float)mean;
    gyro_z_stddev_rads =
        (float)sqrt(squared_deviations / (double)(kCalibrationSamples - 1U));
    return true;
}

bool angle_init(uint8_t imu_address) {
    initialized = false;
    has_previous_sample = false;
    gyro_z_offset_rads = 0.0f;
    gyro_z_stddev_rads = 0.0f;
    previous_gyro_z_rads = 0.0f;
    previous_sample_time_us = 0U;
    angle_rad = 0.0;

    if (!mpu6050_init(imu_address) || !calibrate_gyro_z()) {
        return false;
    }

    initialized = true;
    return true;
}

bool angle_read_deg(double *angle_deg) {
    if (!initialized || angle_deg == NULL) {
        return false;
    }

    float gyro_z_rads;
    if (!mpu6050_read_gyro_z(&gyro_z_rads) || !isfinite(gyro_z_rads)) {
        has_previous_sample = false;
        return false;
    }
    gyro_z_rads -= gyro_z_offset_rads;
    if (fabsf(gyro_z_rads) <= 10.0f * gyro_z_stddev_rads) {
        gyro_z_rads = 0.0f;
    }

    const uint32_t sample_time_us = hal_micros();
    if (has_previous_sample) {
        const uint32_t elapsed_us = sample_time_us - previous_sample_time_us;
        if (elapsed_us == 0U || elapsed_us > kMaxSampleGapUs) {
            has_previous_sample = false;
            return false;
        }

        const double dt_s = (double)elapsed_us / 1000000.0;
        const double mean_gyro_z =
            ((double)previous_gyro_z_rads + (double)gyro_z_rads) * 0.5;
        angle_rad += mean_gyro_z * dt_s;
    }

    previous_gyro_z_rads = gyro_z_rads;
    previous_sample_time_us = sample_time_us;
    has_previous_sample = true;
    *angle_deg = -(angle_rad * kRadiansToDegrees);
    return true;
}
