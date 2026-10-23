#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializa el sensor y entrega la velocidad angular Z en rad/s. */
bool mpu6050_init(uint8_t address);
bool mpu6050_read_gyro_z(float *gyro_z_rads);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MPU6050_H */
