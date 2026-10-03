#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_delay(uint32_t ms);
uint32_t hal_millis(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TIME_H */
