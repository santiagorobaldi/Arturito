#ifndef HAL_SENSORS_H
#define HAL_SENSORS_H

// Capa 1: mismo contrato que en el proyecto de ESP32.
void hal_sensors_init();
float hal_get_distance_cm();
float hal_get_gyro_z_rads();

#endif // HAL_SENSORS_H
