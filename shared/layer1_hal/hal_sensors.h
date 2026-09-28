#ifndef HAL_SENSORS_H
#define HAL_SENSORS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Capa 1: Contrato de sensores ---
void hal_sensors_init();
float hal_get_distance_cm();
float hal_get_gyro_z_rads();

// --- Capa 1: Contrato de tiempo (Reemplaza a Arduino) ---
// Estas funciones evitan que la Capa 2 dependa de "millis()" o "delay()"
void hal_delay(uint32_t ms);
uint32_t hal_millis();

#ifdef __cplusplus
}
#endif

#endif // HAL_SENSORS_H