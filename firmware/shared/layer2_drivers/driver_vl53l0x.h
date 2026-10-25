#ifndef DRIVER_VL53L0X_H
#define DRIVER_VL53L0X_H

#include <stdbool.h>

typedef enum {
    VL53L0X_OK = 0,
    VL53L0X_ERROR_INIT,
    VL53L0X_ERROR_TIMEOUT,
    VL53L0X_ERROR_INVALID_MEASUREMENT,
    VL53L0X_ERROR_COMMUNICATION,
    VL53L0X_ERROR_INTERNAL_API
} vl53l0x_status_t;

typedef enum {
    VL53L0X_PROFILE_DEFAULT = 0,
    VL53L0X_PROFILE_LONG_RANGE,
    VL53L0X_PROFILE_HIGH_SPEED,
    VL53L0X_PROFILE_HIGH_ACCURACY
} vl53l0x_profile_t;

#ifdef __cplusplus
extern "C" {
#endif

vl53l0x_status_t vl53l0x_init(void);
vl53l0x_status_t vl53l0x_set_profile(vl53l0x_profile_t profile);
vl53l0x_status_t vl53l0x_start(void);
vl53l0x_status_t vl53l0x_distance_ready(bool *ready);
vl53l0x_status_t vl53l0x_get_distance(float *distance_cm);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_VL53L0X_H */
