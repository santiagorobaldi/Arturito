#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_PWM_CH_0 = 0,
    HAL_PWM_CH_1,
    HAL_PWM_CH_2,
    HAL_PWM_CH_3,
    HAL_PWM_CH_4,
    HAL_PWM_CH_5,
    HAL_PWM_CH_6,
    HAL_PWM_CH_7,
    HAL_PWM_CH_8,
    HAL_PWM_CH_9,
    HAL_PWM_CH_10,
    HAL_PWM_CH_COUNT
} hal_pwm_pin_t;

bool hal_pwm_init(hal_pwm_pin_t ch, uint32_t freq_hz);
bool hal_pwm_set_duty(hal_pwm_pin_t ch, float duty_pct);   // 0.0 a 100.0

#endif // HAL_PWM_H