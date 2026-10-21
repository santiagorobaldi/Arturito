#ifndef TRACTION_H
#define TRACTION_H
#endif

#include <stdint.h>
#include <stdbool.h>

bool traction_init(void);
bool traction_start_a_b(int pwm_a, int pwm_b);
bool traction_stop(void);