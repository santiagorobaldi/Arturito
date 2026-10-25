#include "driver_vl53l0x.h"
#include "hal_vl53l0x.h"

static bool initialized = false;

static vl53l0x_status_t driver_status(hal_vl53l0x_status_t status)
{
    switch (status) {
    case HAL_VL53L0X_OK:
        return VL53L0X_OK;
    case HAL_VL53L0X_ERROR_INIT:
        return VL53L0X_ERROR_INIT;
    case HAL_VL53L0X_ERROR_TIMEOUT:
        return VL53L0X_ERROR_TIMEOUT;
    case HAL_VL53L0X_ERROR_INVALID_MEASUREMENT:
        return VL53L0X_ERROR_INVALID_MEASUREMENT;
    case HAL_VL53L0X_ERROR_COMMUNICATION:
        return VL53L0X_ERROR_COMMUNICATION;
    default:
        return VL53L0X_ERROR_INTERNAL_API;
    }
}

vl53l0x_status_t vl53l0x_init(void)
{
    hal_vl53l0x_status_t status = hal_vl53l0x_init();
    initialized = (status == HAL_VL53L0X_OK);
    return driver_status(status);
}

vl53l0x_status_t vl53l0x_set_profile(vl53l0x_profile_t profile)
{
    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }
    hal_vl53l0x_profile_t hal_profile;
    switch (profile) {
    case VL53L0X_PROFILE_DEFAULT:
        hal_profile = HAL_VL53L0X_PROFILE_DEFAULT;
        break;
    case VL53L0X_PROFILE_LONG_RANGE:
        hal_profile = HAL_VL53L0X_PROFILE_LONG_RANGE;
        break;
    case VL53L0X_PROFILE_HIGH_SPEED:
        hal_profile = HAL_VL53L0X_PROFILE_HIGH_SPEED;
        break;
    case VL53L0X_PROFILE_HIGH_ACCURACY:
        hal_profile = HAL_VL53L0X_PROFILE_HIGH_ACCURACY;
        break;
    default:
        return VL53L0X_ERROR_INIT;
    }
    return driver_status(hal_vl53l0x_set_profile(hal_profile));
}

vl53l0x_status_t vl53l0x_start(void)
{
    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }
    return driver_status(hal_vl53l0x_start());
}

vl53l0x_status_t vl53l0x_distance_ready(bool *ready)
{
    if (!initialized) {
        *ready = false;
        return VL53L0X_ERROR_INIT;
    }
    return driver_status(hal_vl53l0x_distance_ready(ready));
}

vl53l0x_status_t vl53l0x_get_distance(float *distance_cm) {
    float distance_m;
    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }
    const hal_vl53l0x_status_t status = hal_vl53l0x_get_distance(&distance_m);
    if (status == HAL_VL53L0X_OK) {
        *distance_cm = distance_m * 100.0f;
    }
    return driver_status(status);
}
