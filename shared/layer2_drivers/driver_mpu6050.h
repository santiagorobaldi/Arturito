#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

#ifdef __cplusplus
extern "C" {
#endif

void mpu6050_init(void);
void mpu6050_calibrate(void);
/* Lee HAL e integra. Para super-loop (p. ej. EDU-CIAA). */
void mpu6050_update(void);
/* Integra una muestra ya leida (tarea adquirir en ESP32). */
void mpu6050_update_gyro(float gz_rads_raw);
float mpu6050_get_theta_rad(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_MPU6050_H */
