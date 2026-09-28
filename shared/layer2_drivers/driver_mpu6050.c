#include <math.h>

#include "driver_mpu6050.h"
#include "config.h"
#include "hal_sensors.h"


#ifndef PI
#define PI 3.14159265358979323846f
#endif

// En C puro usamos "static" en lugar de "namespace {}" para variables privadas
static float gyroZ_offset_rads = 0.0f;
static float theta_rad = 0.0f;
static uint32_t tiempo_previo = 0;
static float gz_rads_previo = 0.0f;

void mpu6050_init() {
    // La inicialización real del sensor la hace hal_sensors_init()
}

void mpu6050_calibrate() {
    hal_delay(2000); // Usamos nuestra función agnóstica en lugar de delay()
    float suma = 0.0f;
    for (int i = 0; i < 1000; i++) {
        suma += hal_get_gyro_z_rads();
        hal_delay(2);
    }
    gyroZ_offset_rads = suma / 1000.0f;
    tiempo_previo = hal_millis(); // Usamos hal_millis()
}

void mpu6050_update() {
    float gz_rads = hal_get_gyro_z_rads() - gyroZ_offset_rads;

    uint32_t tiempo_actual = hal_millis();
    float dt = (tiempo_actual - tiempo_previo) / 1000.0f;
    tiempo_previo = tiempo_actual;

    float deadband_rads = 1.0f * (PI / 180.0f);
    if (fabsf(gz_rads) <= deadband_rads) gz_rads = 0.0f;

    // Integración trapezoidal
    float velocidad_promedio = (gz_rads + gz_rads_previo) / 2.0f;
    theta_rad += velocidad_promedio * dt;
    gz_rads_previo = gz_rads;
}

float mpu6050_get_theta_rad() {
    return -theta_rad;
}