#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

// Capa 2: transforma lecturas crudas de registros en un ángulo utilizable.
// Solo depende de hal_i2c.h, no sabe qué placa hay debajo.

void mpu6050_init();

// Bloqueante, ~2s. Correr una sola vez al arranque con el chasis quieto.
void mpu6050_calibrate();

// Llamar periódicamente (cada SAMPLE_PERIOD_MS). Actualiza el ángulo interno.
void mpu6050_update();

// Devuelve el último ángulo integrado, en radianes.
float mpu6050_get_theta_rad();

#endif // DRIVER_MPU6050_H
