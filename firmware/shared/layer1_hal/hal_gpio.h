#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdbool.h>

typedef enum {
    HAL_GPIO_DRV_SLEEP = 0,   // salida: nSLEEP del DRV8833
    HAL_GPIO_DRV_FAULT,       // entrada: nFAULT del DRV8833
    HAL_GPIO_COUNT
} hal_gpio_pin_t;

typedef enum {
    HAL_GPIO_OUT = 0,
    HAL_GPIO_IN,
    HAL_GPIO_IN_PULLUP
} hal_gpio_mode_t;

bool hal_gpio_init(hal_gpio_pin_t p, hal_gpio_mode_t mode);
bool hal_gpio_write(hal_gpio_pin_t p, bool level);
bool hal_gpio_read(hal_gpio_pin_t p, bool *level);

#endif // HAL_GPIO_H