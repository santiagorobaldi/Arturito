#include "distance.h"
#include "driver_vl53l0x.h"
#include "hal_time.h"

/* La aplicacion espera la conversion, valida e interpola la calibracion del ToF. */
#define DISTANCE_READY_TIMEOUT_US 100000U

typedef struct {
    float measured_cm;
    float real_cm;
} CalibrationPoint;

/* High Speed, calibracion sobre carton. */
static const CalibrationPoint calibration[] = {
    { 3.064f,  2.0f },
    { 6.61f,   5.0f },
    {12.51f,  10.0f },
    {17.75f,  15.0f },
    {22.63f,  20.0f },
    {27.38f,  25.0f },
    {31.84f,  30.0f },
    {35.63f,  35.0f },
    {39.92f,  40.0f },
    {43.10f,  45.0f },
    {45.79f,  50.0f },
    {48.10f,  55.0f },
    {50.50f,  60.0f },
    {53.86f,  65.0f }
};

#define CALIBRATION_COUNT (sizeof(calibration) / sizeof(calibration[0]))

static float interpolate_cm(float measured_cm,
                            const CalibrationPoint *a,
                            const CalibrationPoint *b) {
    return a->real_cm +
        (measured_cm - a->measured_cm) *
        (b->real_cm - a->real_cm) /
        (b->measured_cm - a->measured_cm);
}

bool distance_init(void) {
    if (vl53l0x_init() != VL53L0X_OK) {
        return false;
    }
    return vl53l0x_set_profile(VL53L0X_PROFILE_HIGH_SPEED) == VL53L0X_OK;
}

float distance_validate_raw(float raw_cm) {
#if defined(BOARD_ESP32)
    if (!(raw_cm >= calibration[0].measured_cm &&
          raw_cm <= calibration[CALIBRATION_COUNT - 1U].measured_cm)) {
#else
    if (!(raw_cm >= 0.1f && raw_cm <= 150.0f)) {
#endif
        return -1.0f;
    }

    return raw_cm;
}

float distance_get_raw(void) {
    bool ready = false;
    if (vl53l0x_start() != VL53L0X_OK) {
        return -1.0f;
    }

    const uint32_t start_us = hal_micros();
    do {
        if (vl53l0x_distance_ready(&ready) != VL53L0X_OK) {
            return -1.0f;
        }
        if (ready) {
            float distance_cm = -1.0f;
            return (vl53l0x_get_distance(&distance_cm) == VL53L0X_OK)
                ? distance_cm
                : -1.0f;
        }
        hal_delay(1U);
    } while ((uint32_t)(hal_micros() - start_us) < DISTANCE_READY_TIMEOUT_US);

    return -1.0f;
}

float distance_calibrate(float raw_cm) {
    if (distance_validate_raw(raw_cm) < 0.0f) {
        return -1.0f;
    }

    for (unsigned int i = 0U; i < CALIBRATION_COUNT - 1U; ++i) {
        if (raw_cm <= calibration[i + 1U].measured_cm) {
            return interpolate_cm(raw_cm, &calibration[i], &calibration[i + 1U]);
        }
    }

    return -1.0f;
}

float distance_get(void) {
    const float raw_cm = distance_get_raw();
#if defined(BOARD_ESP32)
    return distance_calibrate(raw_cm);
#else
    return distance_validate_raw(raw_cm);
#endif
}
