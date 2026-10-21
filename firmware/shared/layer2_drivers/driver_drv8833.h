#ifndef DRV8833_H
#define DRV8833_H
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum{
    A=0,
    B
}motor;

bool drv8833_init(void);

bool drv8833_start(motor m, int pwm);

bool drv8833_stop(motor m);