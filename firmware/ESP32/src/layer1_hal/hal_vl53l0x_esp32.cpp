#include "hal_vl53l0x.h"

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

namespace {
    Adafruit_VL53L0X sensor = Adafruit_VL53L0X();
    bool vl53l0x_ok = false;
    hal_vl53l0x_diagnostics_t diagnostics = {};
}

void hal_vl53l0x_reset_diagnostics(void) {
    diagnostics = {};
}

void hal_vl53l0x_get_diagnostics(hal_vl53l0x_diagnostics_t *out) {
    if (out != nullptr) {
        *out = diagnostics;
    }
}

bool hal_vl53l0x_init(void) {
    Wire.begin();

    vl53l0x_ok = sensor.begin(
        0x29,
        false,
        &Wire,
        Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED
    );

    return vl53l0x_ok;
}

float hal_vl53l0x_get_distance_cm(void) {
    if (!vl53l0x_ok) {
        ++diagnostics.api_errors;
        return -1.0f;
    }

    VL53L0X_RangingMeasurementData_t measure = {};
    const VL53L0X_Error status = sensor.rangingTest(&measure, false);
    if (status != VL53L0X_ERROR_NONE) {
        ++diagnostics.api_errors;
        return -1.0f;
    }

    if (measure.RangeStatus < 6U) {
        ++diagnostics.range_status_counts[measure.RangeStatus];
    } else {
        ++diagnostics.other_status_count;
    }

    if (measure.RangeMilliMeter == 0xFFFFU) {
        ++diagnostics.sentinel_count;
        return -1.0f;
    }

    if (measure.RangeStatus != 0U) {
        return -1.0f;
    }

    return measure.RangeMilliMeter / 10.0f;
}
