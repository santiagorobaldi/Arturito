#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Reloj compartido: demoras en ms y marcas de tiempo en ms o microsegundos. */
void hal_delay(uint32_t ms);
uint32_t hal_millis(void);
uint32_t hal_micros(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIME_H */
