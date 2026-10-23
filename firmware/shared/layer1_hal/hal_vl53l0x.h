#ifndef HAL_VL53L0X_H
#define HAL_VL53L0X_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    HAL_VL53L0X_OK = 0,
    HAL_VL53L0X_ERROR_INIT,
    HAL_VL53L0X_ERROR_TIMEOUT,
    HAL_VL53L0X_ERROR_INVALID_MEASUREMENT,
    HAL_VL53L0X_ERROR_COMMUNICATION
} hal_vl53l0x_status_t;

typedef enum {
    HAL_VL53L0X_PROFILE_DEFAULT = 0,
    HAL_VL53L0X_PROFILE_LONG_RANGE,
    HAL_VL53L0X_PROFILE_HIGH_SPEED,
    HAL_VL53L0X_PROFILE_HIGH_ACCURACY
} hal_vl53l0x_profile_t;

#ifdef __cplusplus
extern "C" {
#endif

hal_vl53l0x_status_t hal_vl53l0x_init(void);
hal_vl53l0x_status_t hal_vl53l0x_set_profile(hal_vl53l0x_profile_t profile);
hal_vl53l0x_status_t hal_vl53l0x_start(void);
hal_vl53l0x_status_t hal_vl53l0x_distance_ready(bool *ready);
/* La HAL conserva la unidad nativa solicitada por el driver: metros. */
hal_vl53l0x_status_t hal_vl53l0x_get_distance(float *distance_m);

#if defined(BOARD_ESP32)
typedef struct {
    uint16_t api_errors;
    uint16_t range_status_counts[6];
    uint16_t other_status_count;
    uint16_t sentinel_count;
} hal_vl53l0x_diagnostics_t;

void hal_vl53l0x_reset_diagnostics(void);
void hal_vl53l0x_get_diagnostics(hal_vl53l0x_diagnostics_t *diagnostics);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HAL_VL53L0X_H */
