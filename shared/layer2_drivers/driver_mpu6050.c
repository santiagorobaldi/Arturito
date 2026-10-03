#include <math.h>

#include "driver_mpu6050.h"
#include "config.h"
#include "hal_imu.h"
#include "hal_time.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

static float gyroZ_offset_rads = 0.0f;
static float theta_rad = 0.0f;
static uint32_t tiempo_previo = 0;
static float gz_rads_previo = 0.0f;

void mpu6050_init(void) {
}

void mpu6050_calibrate(void) {
    hal_delay(2000);
    float suma = 0.0f;
    for (int i = 0; i < 1000; i++) {
        suma += hal_imu_get_gyro_z_rads();
        hal_delay(2);
    }
    gyroZ_offset_rads = suma / 1000.0f;
    tiempo_previo = hal_millis();
    gz_rads_previo = 0.0f;
    theta_rad = 0.0f;
}

void mpu6050_update_gyro(float gz_rads_raw) {
    float gz_rads = gz_rads_raw - gyroZ_offset_rads;

    uint32_t tiempo_actual = hal_millis();
    float dt = (tiempo_actual - tiempo_previo) / 1000.0f;
    tiempo_previo = tiempo_actual;

    float deadband_rads = 1.0f * (PI / 180.0f);
    if (fabsf(gz_rads) <= deadband_rads) {
        gz_rads = 0.0f;
    }

    float velocidad_promedio = (gz_rads + gz_rads_previo) / 2.0f;
    theta_rad += velocidad_promedio * dt;
    gz_rads_previo = gz_rads;
}

void mpu6050_update(void) {
    mpu6050_update_gyro(hal_imu_get_gyro_z_rads());
}

float mpu6050_get_theta_rad(void) {
    return theta_rad;
}
