
#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* API original: se conserva por compatibilidad. */

/* Espera bloqueante en milisegundos. */
void hal_delay(uint32_t ms);

/* Tiempo del sistema en milisegundos. */
uint32_t hal_millis(void);

/* Tiempo del sistema en microsegundos. */
uint32_t hal_micros(void);


/* API nueva: reloj monotónico de alta resolución. */

/* Inicializa el reloj de alta resolución. */
bool hal_time_init(void);

/* Tiempo en microsegundos, módulo 2^32. */
uint32_t hal_time_us(void);

/* Calcula el tiempo transcurrido desde start_us. */
uint32_t hal_time_elapsed_us(uint32_t start_us);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIME_H */
