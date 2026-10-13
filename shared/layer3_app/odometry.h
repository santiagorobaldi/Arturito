#ifndef ODOMETRY_H
#define ODOMETRY_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Inicializa la estimacion angular relativa de un unico eje.
 * initial_angle_rad debe ser finito; deadband_rads, finito y no negativo.
 * Establece la referencia angular y borra el historial de integracion.
 * Devuelve false ante parametros invalidos, conservando el estado anterior. */
bool odometry_init(float initial_angle_rad, float deadband_rads);

/** Actualiza el angulo con velocidad angular en rad/s e instante en ms.
 * Requiere inicializacion. Cada muestra se entrega una vez y en orden,
 * utilizando un mismo reloj. La velocidad debe ser finita.
 * Aplica zona muerta; la primera muestra establece referencia sin integrar.
 * Luego integra el promedio de velocidades consecutivas.
 * Admite desbordamiento del contador si el intervalo es menor que 2^31 ms.
 * Rechaza tiempos repetidos o intervalos mayores o iguales a 2^31 ms.
 * Ante fallo conserva el estado anterior. */
bool odometry_update(float angular_velocity_rads, uint32_t sample_time_ms);

/** Devuelve el angulo relativo en radianes, con signo y vueltas acumuladas.
 * Requiere inicializacion exitosa. */
float odometry_get_angle(void);

/** Establece una referencia angular finita y borra el historial temporal.
 * Conserva la zona muerta. Requiere inicializacion.
 * Devuelve false ante fallo, conservando el estado anterior. */
bool odometry_reset(float angle_rad);

/** Descarta el historial de integracion conservando el angulo.
 * La siguiente muestra establece una nueva referencia temporal.
 * Requiere inicializacion; devuelve false si falta esa condicion. */
bool odometry_restart_tracking(void);

#ifdef __cplusplus
}
#endif
#endif
