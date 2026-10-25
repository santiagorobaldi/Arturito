#include "traction.h"
#include "driver_drv8833.h"
#include "driver_encoder.h"
#include "pi.h"


#include <stdint.h>
#include <stdbool.h>

#Velocidad angular máxima y tasa
#define W_MAX (VEL_MAX / WHEEL_RADIUS_M) //rad/s
#define W_RATE (VEL_RAMP/ WHEEL_RADIUS_M) 

#define TRACTION_TS_S (TRACTION_TICK_MS / 1000.0f)

#Signos de Motores
#define MOTOR_LEFT_SIGN (1.0f)
#define MOTOR_RIGHT_SIGN (-1.0f)

//Signos de Encoders 
#define ENC_LEFT_SIGN (1.0f)
#define ENC_RIGHT_SIGN (1.0f)

//Parametros PI
#define MOTOR_L_K 0.2f
#define MOTOR_R_K 0.2f

#define MOTOR_L_TAU_S 0.1f
#define MOTOR_R_TAU_S 0.1f

#define TAU_LC_S 0.1f

#define MOTOR_L_KP (MOTOR_L_TAU_S / (MOTOR_L_K  * TAU_LC_S))
#define MOTOR_R_KP (MOTOR_R_TAU_S / (MOTOR_R_K  * TAU_LC_S))

#define MOTOR_L_KI (MOTOR_L_KP/ MOTOR_L_TAU_S)
#define MOTOR_R_KI (MOTOR_R_KP/ MOTOR_R_TAU_S)


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

typedef struct{
    float prev;
    float act;
}angle_t;

static angle_t enc_left_angle = {0,0};
static angle_t enc_right_angle = {0,0};

static float w_left_read = 0;
static float w_right_read = 0;

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

bool traction_tick(void){
    float left_angle_rad = 0;
    float right_angle_rad = 0;

    encoder_get_angle_rad (ENC_IZQ, &left_angle_rad);
    encoder_get_angle_rad (ENC_DER, &right_angle_rad);

    w_left_read = (left_angle_rad - enc_left_angle.prev) / TRACTION_TS_S;
    w_right_read = (right_angle_rad - enc_right_angle.prev) / TRACTION_TS_S;

    enc_left_angle.prev = left_angle_rad;
    enc_right_angle.prev = right_angle_rad;

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

    float duty = pi_update(&pi_left, r_left ,w_left_read, TRACTION_TS_S);
    bool ok = drv8833_set_pwm(MOTOR_A,duty);

    duty = pi_update(&pi_right, r_right ,w_right_read, TRACTION_TS_S);
    ok &= drv8833_set_pwm(MOTOR_B,duty);

    return ok;
}

