#include "hal_vl53l0x.h"

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

namespace {
Adafruit_VL53L0X sensor;
bool initialized = false;
hal_vl53l0x_diagnostics_t diagnostics = {};

hal_vl53l0x_status_t map_status(VL53L0X_Error status) {
    if (status == VL53L0X_ERROR_NONE) {
        return HAL_VL53L0X_OK;
    }
    if (status == VL53L0X_ERROR_TIME_OUT) {
        return HAL_VL53L0X_ERROR_TIMEOUT;
    }
    if (status == VL53L0X_ERROR_RANGE_ERROR) {
        return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    }
    return HAL_VL53L0X_ERROR_COMMUNICATION;
}

bool apply_profile(hal_vl53l0x_profile_t profile) {
    uint16_t sigma;
    uint8_t pre_period;
    uint8_t final_period;
    uint32_t budget_us;
    bool ignore_enabled = true;
    bool configure_ignore_threshold = true;

    switch (profile) {
    case HAL_VL53L0X_PROFILE_DEFAULT:
        sigma = 18U; pre_period = 14U; final_period = 10U;
        budget_us = 33000U;
        break;
    case HAL_VL53L0X_PROFILE_LONG_RANGE:
        sigma = 60U; pre_period = 18U; final_period = 14U;
        budget_us = 33000U;
        break;
    case HAL_VL53L0X_PROFILE_HIGH_SPEED:
        sigma = 32U; pre_period = 14U; final_period = 10U;
        budget_us = 30000U;
        configure_ignore_threshold = false;
        break;
    case HAL_VL53L0X_PROFILE_HIGH_ACCURACY:
        sigma = 18U; pre_period = 14U; final_period = 10U;
        budget_us = 200000U;
        ignore_enabled = false;
        break;
    default:
        return false;
    }

    const FixPoint1616_t sigma_value = (FixPoint1616_t)(sigma * 65536UL);
    const FixPoint1616_t signal_value = (FixPoint1616_t)(
        (profile == HAL_VL53L0X_PROFILE_LONG_RANGE ? 0.10f : 0.25f) * 65536.0f);
    const FixPoint1616_t ignore_value = (FixPoint1616_t)(1.5f * 0.023f * 65536.0f);

    if (!sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1) ||
        !sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1) ||
        !sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, sigma_value) ||
        !sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, signal_value)) {
        return false;
    }
    if (configure_ignore_threshold &&
        (!sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, ignore_value) ||
         !sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
                                     ignore_enabled ? 1U : 0U))) {
        return false;
    }
    return sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, pre_period) &&
           sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, final_period) &&
           sensor.setMeasurementTimingBudgetMicroSeconds(budget_us);
}
}

void hal_vl53l0x_reset_diagnostics(void) {
    diagnostics = {};
}

void hal_vl53l0x_get_diagnostics(hal_vl53l0x_diagnostics_t *out) {
    if (out != nullptr) {
        *out = diagnostics;
    }
}

hal_vl53l0x_status_t hal_vl53l0x_init(void) {
    initialized = false;
    Wire.begin();
    if (!sensor.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_SPEED)) {
        ++diagnostics.api_errors;
        return HAL_VL53L0X_ERROR_INIT;
    }
    if (sensor.setDeviceMode(VL53L0X_DEVICEMODE_SINGLE_RANGING, false) != VL53L0X_ERROR_NONE) {
        ++diagnostics.api_errors;
        return HAL_VL53L0X_ERROR_INIT;
    }
    initialized = true;
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_set_profile(hal_vl53l0x_profile_t profile) {
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;
    if (!apply_profile(profile)) {
        ++diagnostics.api_errors;
        return HAL_VL53L0X_ERROR_INIT;
    }
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_start(void) {
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;
    const hal_vl53l0x_status_t status = map_status(sensor.startMeasurement(false));
    if (status != HAL_VL53L0X_OK) ++diagnostics.api_errors;
    return status;
}

hal_vl53l0x_status_t hal_vl53l0x_distance_ready(bool *ready) {
    if (ready == nullptr) return HAL_VL53L0X_ERROR_COMMUNICATION;
    *ready = false;
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;

    const bool complete = sensor.isRangeComplete();
    const hal_vl53l0x_status_t status = map_status(sensor.Status);
    if (status == HAL_VL53L0X_OK) {
        *ready = complete;
    } else {
        ++diagnostics.api_errors;
    }
    return status;
}

hal_vl53l0x_status_t hal_vl53l0x_get_distance(float *distance_m) {
    if (distance_m == nullptr) return HAL_VL53L0X_ERROR_COMMUNICATION;
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;

    VL53L0X_RangingMeasurementData_t measurement = {};
    hal_vl53l0x_status_t status = map_status(sensor.getRangingMeasurement(&measurement, false));
    if (status == HAL_VL53L0X_OK) {
        status = map_status(sensor.clearInterruptMask(false));
    }
    if (status != HAL_VL53L0X_OK) {
        ++diagnostics.api_errors;
        return status;
    }

    if (measurement.RangeStatus < 6U) {
        ++diagnostics.range_status_counts[measurement.RangeStatus];
    } else {
        ++diagnostics.other_status_count;
    }
    if (measurement.RangeMilliMeter == 0xFFFFU) {
        ++diagnostics.sentinel_count;
        return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    }
    if (measurement.RangeStatus != 0U) {
        return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    }

    *distance_m = (float)measurement.RangeMilliMeter / 1000.0f;
    return HAL_VL53L0X_OK;
}
