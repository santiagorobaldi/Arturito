#include "hal_vl53l0x.h"
#include "vl53l0x_api.h"

#include <string.h>

static VL53L0X_Dev_t sensor;
static bool initialized;

static hal_vl53l0x_status_t map_status(VL53L0X_Error status) {
    if (status == VL53L0X_ERROR_NONE) return HAL_VL53L0X_OK;
    if (status == VL53L0X_ERROR_TIME_OUT) return HAL_VL53L0X_ERROR_TIMEOUT;
    if (status == VL53L0X_ERROR_RANGE_ERROR) return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    return HAL_VL53L0X_ERROR_COMMUNICATION;
}

static bool apply_profile(hal_vl53l0x_profile_t profile) {
    uint16_t sigma;
    uint32_t signal_rate;
    uint8_t pre_period;
    uint8_t final_period;
    uint32_t budget_us;
    bool ignore_enabled = true;

    switch (profile) {
    case HAL_VL53L0X_PROFILE_DEFAULT:
        sigma = 18U; signal_rate = 16384U; pre_period = 14U; final_period = 10U;
        budget_us = 33000U;
        break;
    case HAL_VL53L0X_PROFILE_LONG_RANGE:
        sigma = 60U; signal_rate = 6553U; pre_period = 18U; final_period = 14U;
        budget_us = 33000U;
        break;
    case HAL_VL53L0X_PROFILE_HIGH_SPEED:
        sigma = 32U; signal_rate = 16384U; pre_period = 14U; final_period = 10U;
        budget_us = 30000U;
        break;
    case HAL_VL53L0X_PROFILE_HIGH_ACCURACY:
        sigma = 18U; signal_rate = 16384U; pre_period = 14U; final_period = 10U;
        budget_us = 200000U;
        ignore_enabled = false;
        break;
    default:
        return false;
    }

    VL53L0X_Error status = VL53L0X_SetLimitCheckEnable(
        &sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckEnable(
            &sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckValue(&sensor,
            VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(sigma * 65536UL));
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckValue(&sensor,
            VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)signal_rate);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckValue(&sensor,
            VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD,
            (FixPoint1616_t)(1.5f * 0.023f * 65536.0f));
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckEnable(&sensor,
            VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, ignore_enabled ? 1U : 0U);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetVcselPulsePeriod(&sensor,
            VL53L0X_VCSEL_PERIOD_PRE_RANGE, pre_period);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetVcselPulsePeriod(&sensor,
            VL53L0X_VCSEL_PERIOD_FINAL_RANGE, final_period);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&sensor, budget_us);
    }
    return status == VL53L0X_ERROR_NONE;
}

hal_vl53l0x_status_t hal_vl53l0x_init(void) {
    uint8_t vhv_settings;
    uint8_t phase_cal;
    uint32_t spad_count;
    uint8_t aperture_spads;
    VL53L0X_DeviceInfo_t info;

    initialized = false;
    memset(&sensor, 0, sizeof(sensor));
    sensor.I2cDevAddr = 0x52;
    sensor.comms_type = 1;
    sensor.comms_speed_khz = 400;
    sensor.i2cBus = I2C0;

    /* I2C0 lo inicializa la aplicacion al iniciar la IMU. */
    if (VL53L0X_DataInit(&sensor) != VL53L0X_ERROR_NONE ||
        VL53L0X_GetDeviceInfo(&sensor, &info) != VL53L0X_ERROR_NONE ||
        VL53L0X_StaticInit(&sensor) != VL53L0X_ERROR_NONE ||
        VL53L0X_PerformRefCalibration(&sensor, &vhv_settings, &phase_cal) != VL53L0X_ERROR_NONE ||
        VL53L0X_PerformRefSpadManagement(&sensor, &spad_count, &aperture_spads) != VL53L0X_ERROR_NONE ||
        VL53L0X_SetDeviceMode(&sensor, VL53L0X_DEVICEMODE_SINGLE_RANGING) != VL53L0X_ERROR_NONE) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    initialized = true;
    if (!apply_profile(HAL_VL53L0X_PROFILE_DEFAULT)) {
        initialized = false;
        return HAL_VL53L0X_ERROR_INIT;
    }
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_set_profile(hal_vl53l0x_profile_t profile) {
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;
    return apply_profile(profile) ? HAL_VL53L0X_OK : HAL_VL53L0X_ERROR_INIT;
}

hal_vl53l0x_status_t hal_vl53l0x_start(void) {
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;
    return map_status(VL53L0X_StartMeasurement(&sensor));
}

hal_vl53l0x_status_t hal_vl53l0x_distance_ready(bool *ready) {
    if (ready == NULL) return HAL_VL53L0X_ERROR_COMMUNICATION;
    *ready = false;
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;
    uint8_t data_ready = 0U;
    const hal_vl53l0x_status_t status = map_status(
        VL53L0X_GetMeasurementDataReady(&sensor, &data_ready));
    if (status == HAL_VL53L0X_OK) *ready = data_ready != 0U;
    return status;
}

hal_vl53l0x_status_t hal_vl53l0x_get_distance(float *distance_m) {
    if (distance_m == NULL) return HAL_VL53L0X_ERROR_COMMUNICATION;
    if (!initialized) return HAL_VL53L0X_ERROR_INIT;

    VL53L0X_RangingMeasurementData_t measurement = {0};
    hal_vl53l0x_status_t status = map_status(
        VL53L0X_GetRangingMeasurementData(&sensor, &measurement));
    if (status != HAL_VL53L0X_OK) return status;
    status = map_status(VL53L0X_ClearInterruptMask(&sensor, 0U));
    if (status != HAL_VL53L0X_OK) return status;
    if (measurement.RangeStatus != 0U) return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;

    *distance_m = (float)measurement.RangeMilliMeter / 1000.0f;
    return HAL_VL53L0X_OK;
}
