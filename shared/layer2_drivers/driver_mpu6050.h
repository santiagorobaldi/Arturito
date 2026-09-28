#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

#ifdef __cplusplus
extern "C" {
#endif

// Capa 2: transforma lecturas crudas de registros en un ángulo utilizable.
void mpu6050_init();
void mpu6050_calibrate();
void mpu6050_update();
float mpu6050_get_theta_rad();

#ifdef __cplusplus
}
#endif

#endif // DRIVER_MPU6050_H