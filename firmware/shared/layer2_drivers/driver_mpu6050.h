#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Inicializa una MPU6050 en una direccion I2C de 7 bits.
 * Establece offsets en cero. Requiere direccion valida (0x68U o 0x69U) y acceso I2C funcional.
 * Devuelve true cuando el dispositivo queda disponible. False si la direccion es invalida o falla la inicializacion.
 * Los rangos de adquisicion se especifican en la implementacion de acceso. */
bool mpu6050_init(uint8_t address);

/** Lee velocidad angular Z en rad/s y corrige su offset.
 * Requiere inicializacion y un puntero valido.
 * Devuelve true para una muestra valida; ante fallo conserva *gyro_z_rads.
 * La llamada es bloqueante. */
bool mpu6050_read_gyro_z(float *gyro_z_rads);

/** Lee gyro Z, corrige su bias e integra el angulo desde mpu6050_init.
 * Devuelve el angulo acumulado en grados. Si falla una lectura o el intervalo
 * excede el maximo configurado, devuelve false y conserva el angulo en salida;
 * la siguiente muestra valida reinicia la referencia temporal. */
bool mpu6050_read_angle_deg(double *angle_deg);

/** Estima el offset residual de gyro Z,
 * promediando lecturas con el dispositivo inmovil.
 * sample_count e interval_ms deben ser mayores que cero.
 * Espera interval_ms entre lecturas; la llamada es bloqueante.
 * El llamador asegura inmovilidad. Si tiene exito reemplaza el offset;
 * ante fallo conserva la calibracion anterior. */
bool mpu6050_calibrate_gyro_z(uint32_t sample_count, uint32_t interval_ms);

#ifdef __cplusplus
}
#endif
#endif
