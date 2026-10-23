#include "driver_vl53l0x.h"

static bool initialized = false;

hal_vl53l0x_status_t vl53l0x_init(void)
{
    hal_vl53l0x_status_t status = hal_vl53l0x_init();
    initialized = (status == HAL_VL53L0X_OK);
    return status;
}

hal_vl53l0x_status_t vl53l0x_set_profile(hal_vl53l0x_profile_t profile)
{
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }
    return hal_vl53l0x_set_profile(profile);
}

hal_vl53l0x_status_t vl53l0x_start(void)
{
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }
    return hal_vl53l0x_start();
}

hal_vl53l0x_status_t vl53l0x_distance_ready(bool *ready)
{
    if (!initialized) {
        *ready = false;
        return HAL_VL53L0X_ERROR_INIT;
    }
    return hal_vl53l0x_distance_ready(ready);
}

hal_vl53l0x_status_t vl53l0x_get_distance(float *distance_cm) {
    float distance_m;
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }
    const hal_vl53l0x_status_t status = hal_vl53l0x_get_distance(&distance_m);
    if (status == HAL_VL53L0X_OK) {
        *distance_cm = distance_m * 100.0f;
    }
    return status;
}
