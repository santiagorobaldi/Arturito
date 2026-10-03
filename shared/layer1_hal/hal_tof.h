#ifndef HAL_TOF_H
#define HAL_TOF_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Capa 1: VL53L0X. Distancia en metros; invalida = -1. No valida rango de app. */
bool hal_tof_init(void);
float hal_tof_get_distance_m(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TOF_H */
