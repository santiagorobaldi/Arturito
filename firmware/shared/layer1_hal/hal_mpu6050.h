#ifndef HAL_MPU6050_H
#define HAL_MPU6050_H

#include "driver_mpu6050.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Prepara el acceso a una MPU6050 y configura su adquisicion.
 * address es una direccion I2C de 7 bits.
 * Devuelve true al completar la inicializacion; ante fallo inhabilita lectura.
 * La implementacion documenta rangos de medicion y limites de espera. */
bool hal_mpu6050_init(uint8_t address);

/** Adquiere velocidad angular Z en rad/s.
 * Requiere inicializacion y puntero valido. La llamada es bloqueante.
 * Devuelve true al completar la adquisicion; ante un fallo detectado
 * conserva *gyro_z_rads. */
bool hal_mpu6050_read_gyro_z(float *gyro_z_rads);

#ifdef __cplusplus
}
#endif
#endif
