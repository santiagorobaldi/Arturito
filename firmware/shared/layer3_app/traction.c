#include "traction.h"
#include "driver_drv8833.h"
#include "driver_as5600.h"

#include <stdint.h>
#include <stdbool.h>

#define W_MAX (VEL_MAX / WHEEL_RADIUS_M) //rad/s
#define W_RATE (VEL_RAMP/ WHEEL_RADIUS_M) 

#define RAMP_DT (TRACTION_TICK_MS / 1000.0f)

#define MOTOR_LEFT_SIGN (1.0f)
#define MOTOR_RIGHT_SIGN (-1.0f)

// Velocidad angular
typedef struct{
    float left;
    float right;
}wheel_w_t;

// Rampa slew-rate
typedef struct{
    float value; // 
    float rate; // pendiente de rampa
}ramp_t;

static wheel_w_t ref = {0,0};

static ramp_t ramp_left = {0,W_RATE};
static ramp_t ramp_right = {0,W_RATE};

static volatile bool stopped = true;

static float ramp_update(ramp_t *r, float target, float dt) {
    float max_step = r->rate * dt;
    float diff = target - r->value;
    if (diff >  max_step) diff =  max_step;
    if (diff < -max_step) diff = -max_step;
    r->value += diff;
    return r->value;
}

static float clamp (float x, float lim){
    if (x > lim) return lim;
    if (x < -lim) return -lim;
    return x;
}

static bool pid(drv8833_motor_t m, as5600_t n, float w_ref){  
//TODO: Conversión de W->PWM con PI 
    float duty = 100.0f * w_ref / W_MAX;
    return drv8833_set_pwm(m,duty);
}

bool traction_init(void){
    return drv8833_init();
}

bool traction_resume(void){
    stopped = false;
    return true;
}

bool traction_stop(void){
    stopped = true;

    return true;
}

bool traction_set_ref(float w_left, float w_right){
    ref.left = clamp(w_left, W_MAX);
    ref.right = clamp(w_right, W_MAX); 
    return true;
}

bool traction_tick(void){
    if (drv8833_fault()) stopped = true;

    if (stopped) {
        ramp_left.value = 0.0f;
        ramp_right.value = 0.0f;
        //reset pi
        return drv8833_break();
    }

    float r_left = ramp_update(&ramp_left,ref.left * MOTOR_LEFT_SIGN, RAMP_DT);
    float r_right = ramp_update(&ramp_right,ref.right * MOTOR_RIGHT_SIGN, RAMP_DT);

    pid(MOTOR_A, ENC_A, r_left);
    pid(MOTOR_B, ENC_B, r_right);
    return true;
}

