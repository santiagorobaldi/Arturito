#ifndef TRACTION_H
#define TRACTION_H


#include <stdint.h>
#include <stdbool.h>

#define VEL_MAX 0.35f
#define VEL_RAMP 0.25f
#define TRACTION_TICK_MS 10 //Debería ir en un config
#define WHEEL_RADIUS_M 0.022f

bool traction_init(void);
bool traction_resume(void);
bool traction_set_ref(float w_left, float w_right);
bool traction_tick(void);
bool traction_stop(void);

#endif