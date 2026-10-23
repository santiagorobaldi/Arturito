#ifndef DRV8833_H
#define DRV8833_H

#include <stdbool.h>

#define MAX_PWM 1000   // 0..1000 = 0..100.0 %, resolución 0.1 %

typedef enum {
    MOTOR_A = 0,       // rueda izquierda
    MOTOR_B,           // rueda derecha
    MOTOR_COUNT
} drv8833_motor_t;

bool drv8833_init(void);
bool drv8833_set_pwm(drv8833_motor_t m, int pwm);   // -MAX_PWM..MAX_PWM
bool drv8833_stop(void);
bool drv8833_fault(void);                           // true si hay falla

#endif