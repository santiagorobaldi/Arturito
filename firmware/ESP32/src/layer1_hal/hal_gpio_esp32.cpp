#include <Arduino.h>
extern "C" {
#include "hal_gpio.h"
}
#include "esp32_board_config.h"

static const uint8_t pins[HAL_GPIO_COUNT] = {
    BOARD_DRV_SLEEP_GPIO, BOARD_DRV_FAULT_GPIO
};
static bool ready[HAL_GPIO_COUNT];
static hal_gpio_mode_t modes[HAL_GPIO_COUNT];

extern "C" bool hal_gpio_init(hal_gpio_pin_t p, hal_gpio_mode_t mode)
{
    if ((unsigned)p >= HAL_GPIO_COUNT) return false;
    switch (mode) {
    case HAL_GPIO_OUT:
        digitalWrite(pins[p], LOW);
        pinMode(pins[p], OUTPUT);
        break;
    case HAL_GPIO_IN: pinMode(pins[p], INPUT); break;
    case HAL_GPIO_IN_PULLUP: pinMode(pins[p], INPUT_PULLUP); break;
    default: return false;
    }
    modes[p] = mode;
    ready[p] = true;
    return true;
}

extern "C" bool hal_gpio_write(hal_gpio_pin_t p, bool level)
{
    if ((unsigned)p >= HAL_GPIO_COUNT || !ready[p] || modes[p] != HAL_GPIO_OUT)
        return false;
    digitalWrite(pins[p], level ? HIGH : LOW);
    return true;
}

extern "C" bool hal_gpio_read(hal_gpio_pin_t p, bool *level)
{
    if ((unsigned)p >= HAL_GPIO_COUNT || !ready[p] || level == nullptr)
        return false;
    *level = digitalRead(pins[p]) != LOW;
    return true;
}
