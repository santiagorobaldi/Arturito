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

/** Adquiere una muestra conjunta en ejes del sensor.
 * Aceleracion en m/s2 con gravedad y velocidad angular en rad/s,
 * Incluye las correcciones propias de la biblioteca de adquisicion.
 * La implementacion documenta cualquier calibracion automatica.
 * Requiere inicializacion y sample valido. La llamada es bloqueante.
 * Devuelve true al completar la adquisicion; ante un fallo detectado
 * conserva *sample. La deteccion de fallos depende del soporte de acceso. */
bool hal_mpu6050_read(mpu6050_sample_t *sample);

#ifdef __cplusplus
}
#endif
#endif
