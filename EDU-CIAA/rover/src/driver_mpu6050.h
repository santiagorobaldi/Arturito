#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

void mpu6050_calibrate();
void mpu6050_update();
float mpu6050_get_theta_rad();

#endif // DRIVER_MPU6050_H
