#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Mediciones en ejes del sensor: aceleracion en m/s2 (incluye gravedad)
 * y velocidad angular en rad/s. */
typedef struct {
    float accel_x_mss, accel_y_mss, accel_z_mss;
    float gyro_x_rads, gyro_y_rads, gyro_z_rads;
} mpu6050_sample_t;

/** Inicializa una MPU6050 en una direccion I2C de 7 bits.
 * Establece offsets en cero si tiene exito; ante fallo inhabilita la lectura.
 * Devuelve true cuando el dispositivo queda disponible.
 * Los rangos de adquisicion se especifican en la implementacion de acceso. */
bool mpu6050_init(uint8_t address);

/** Adquiere los seis componentes y corrige el offset del giroscopio.
 * Requiere inicializacion y un puntero valido.
 * Devuelve true para una muestra valida; ante fallo conserva *sample.
 * La llamada es bloqueante. */
bool mpu6050_read(mpu6050_sample_t *sample);

/** Estima offsets residuales X/Y/Z sobre las mediciones adquiridas,
 * promediando lecturas con el dispositivo inmovil.
 * sample_count e interval_ms deben ser mayores que cero.
 * Espera interval_ms entre lecturas; la llamada es bloqueante.
 * El llamador asegura inmovilidad. Si tiene exito reemplaza los offsets;
 * ante fallo conserva la calibracion anterior. */
bool mpu6050_calibrate_gyro(uint32_t sample_count, uint32_t interval_ms);

#ifdef __cplusplus
}
#endif
#endif
