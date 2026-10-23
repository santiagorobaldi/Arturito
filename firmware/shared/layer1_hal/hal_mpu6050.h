#ifndef HAL_MPU6050_H
#define HAL_MPU6050_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Interfaz comun: aceleracion en m/s^2 y velocidad angular en rad/s. */
typedef struct {
    float accel_x_mss, accel_y_mss, accel_z_mss;
    float gyro_x_rads, gyro_y_rads, gyro_z_rads;
} hal_mpu6050_sample_t;

bool hal_mpu6050_init(uint8_t address);
bool hal_mpu6050_read(hal_mpu6050_sample_t *sample);
bool hal_mpu6050_read_gyro_z(float *gyro_z_rads);

#ifdef __cplusplus
}
#endif

#endif /* HAL_MPU6050_H */
