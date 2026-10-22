#ifndef TRACTION_H
#define TRACTION_H


#include <stdint.h>
#include <stdbool.h>

#define VEL_SATURATION 1000

bool traction_init(void);
bool traction_set_ref(float vel_a, float vel_b);
bool traction_tick(void);
bool traction_stop(void);

#endif