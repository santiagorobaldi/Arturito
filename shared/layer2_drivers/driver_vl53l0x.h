#ifndef DRIVER_VL53L0X_H
#define DRIVER_VL53L0X_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool vl53l0x_init(void);
float vl53l0x_validate_m(float distancia_m);
float vl53l0x_get_distance_m(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_VL53L0X_H */
