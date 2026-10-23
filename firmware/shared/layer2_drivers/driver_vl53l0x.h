#ifndef DRIVER_VL53L0X_H
#define DRIVER_VL53L0X_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool vl53l0x_init(void);
float vl53l0x_validate_cm(float distancia_cm);
float vl53l0x_get_raw_distance_cm(void);
float vl53l0x_calibrate_cm(float distancia_cruda_cm);
float vl53l0x_get_distance_cm(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_VL53L0X_H */
