#ifndef DRV8833_H
#define DRV8833_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PWM 1000 //resolución de 0.1

typedef enum{
    MOTOR_A=0,
    MOTOR_B,
    MOTOR_COUNT
}drv8833_t;

bool drv8833_init(void);

bool drv8833_set_pwm(drv8833_t m, int pwm);

bool drv8833_stop(void);

#endif