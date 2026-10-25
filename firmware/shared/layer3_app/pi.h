#ifndef PI_H
#define PI_H
 
#include <stdbool.h>
 
typedef struct {
    float kp;
    float ki;
    float integral;
} pi_t;

bool pi_reset (pi_t *pi);

float pi_update( pi_t *pi, float ref, float meas, float dt_s);

#endif