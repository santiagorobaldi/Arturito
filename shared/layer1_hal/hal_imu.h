#ifndef HAL_IMU_H
#define HAL_IMU_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Capa 1: MPU6050. Devuelve omega_z en rad/s. No calibra ni integra. */
bool hal_imu_init(void);
float hal_imu_get_gyro_z_rads(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_IMU_H */
