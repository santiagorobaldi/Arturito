#include "pi.h"

#define MAX_PWM 100.0f

bool pi_reset(pi_t *pi){
    pi->integral =0.0f;
    return true;
}

float pi_update(pi_t *pi, float ref, float meas, float dt_s){
    
    if (dt_s <= 0.0f) return 0.0f;

    float e = ref - meas;
    float i_new = pi->integral + e * dt_s;
    float u = pi->kp * e + pi->ki * i_new;
    pi->integral = i_new;

    if (u > MAX_PWM) u = MAX_PWM;
    if (u < -MAX_PWM) u = -MAX_PWM;

    //TODO: Windup

    return u;
}