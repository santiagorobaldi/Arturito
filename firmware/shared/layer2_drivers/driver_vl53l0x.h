#ifndef DRIVER_VL53L0X_H
#define DRIVER_VL53L0X_H

#include "hal_vl53l0x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Mismo contrato que HAL: init -> [perfil] -> start -> ready -> distance.
 * Punteros de salida validos. Consultar ready hasta true antes de recoger.
 */
hal_vl53l0x_status_t vl53l0x_init(void);
hal_vl53l0x_status_t vl53l0x_set_profile(hal_vl53l0x_profile_t profile);
hal_vl53l0x_status_t vl53l0x_start(void);
hal_vl53l0x_status_t vl53l0x_distance_ready(bool *ready);
hal_vl53l0x_status_t vl53l0x_get_distance(float *distance_m);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_VL53L0X_H */
