#ifndef DRV8833_H
#define DRV8833_H

#include <stdint.h>
#include <stdbool.h>

typedef enum{
    MOTOR_A=0,
    MOTOR_B,
    MOTOR_COUNT
}drv8833_t;

bool drv8833_init(void);

bool drv8833_set_pwm(drv8833_t m, int pwm);

bool drv8833_stop(drv8833_t m);

#endif