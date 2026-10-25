#include "traction.h"
#include "driver_drv8833.h"
#include "driver_as5600.h"
#include "pi.h"


#include <stdint.h>
#include <stdbool.h>

#define W_MAX (VEL_MAX / WHEEL_RADIUS_M) //rad/s
#define W_RATE (VEL_RAMP/ WHEEL_RADIUS_M) 

#define TRACTION_TS_S (TRACTION_TICK_MS / 1000.0f)

#define MOTOR_LEFT_SIGN (1.0f)
#define MOTOR_RIGHT_SIGN (-1.0f)

#define MOTOR_L_KP 
#define MOTOR_R_KP

#define MOTOR_L_KI (MOTOR_L_KP/)
#define MOTOR_R_KI (MOTOR_R_KP/)

#define ENC_LEFT_SIGN (1.0f)
#define ENC_RIGHT_SIGN (1.0f)

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

static pi_t pi_left = {MOTOR_L_KP, MOTOR_L_KI, 0.0f};
static pi_t pi_right = {MOTOR_R_KP, MOTOR_R_KI, 0.0f};

static ramp_t ramp_left = {0,W_RATE};
static ramp_t ramp_right = {0,W_RATE};

static float enc_left_meas = 0;
static float enc_right_meas = 0;

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

static bool wheel_loop (drv8833_motor_t m, pi_t *pi, as5600_t enc, float enc_sign, float w_ref){
    float w_meas = enc_sign * encoder_get_speed(enc);
    float duty = pi_update(pi, w_ref, w_meas, TRACTION_TS_S);
    return drv8833_set_pwm(m,duty);
}

bool traction_tick(void){
    enc_left_meas = ENC_LEFT_SIGN * encoder_get_speed (ENC_LEFT);
    enc_right_meas = ENC_RIGHT_SIGN * encoder_get_speed (ENC_RIGHT);

    if (drv8833_fault()) stopped = true;

    if (stopped) {
        ramp_left.value = 0.0f;
        ramp_right.value = 0.0f;
        pi_reset(&pi_left);
        pi_reset(&pi_right);
        return drv8833_brake();
    }

    float r_left = ramp_update(&ramp_left,ref.left * MOTOR_LEFT_SIGN, TRACTION_TS_S);
    float r_right = ramp_update(&ramp_right,ref.right * MOTOR_RIGHT_SIGN, TRACTION_TS_S);

    float duty = pi_update(&pi_left, r_left ,enc_left_meas, TRACTION_TS_S);
    drv8833_set_pwm(MOTOR_A,duty);

    duty = pi_update(&pi_right, r_right ,enc_right_meas, TRACTION_TS_S);
    drv8833_set_pwm(MOTOR_B,duty);

    bool ok = wheel_loop(MOTOR_A, pi_left, ENC_LEFT, ENC_LEFT_SIGN, r_left);
    ok &= wheel_loop(MOTOR_B, pi_right, ENC_RIGHT, ENC_RIGHT_SIGN, r_right);
    return ok;
}

