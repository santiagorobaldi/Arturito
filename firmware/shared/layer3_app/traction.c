#include "traction.h"
#include "driver_drv8833.h"

//#include "driver_as5600.h"

#include <stdint.h>
#include <stdbool.h>



typedef struct{
    float left;
    float right;
}motor_vel_t;

static motor_vel_t ref = {0,0};

static bool stopped = 1;

static bool pid(drv8833_motor_t m, as5600_t n, float vel){  
    //conversión de vel -> pwm
    float pwm;
    if (vel > VEL_SATURATION) pwm = VEL_SATURATION;
    else if (vel < -VEL_SATURATION) pwm = -VEL_SATURATION;

    return drv8833_set_pwm(m,pwm);
}

bool traction_init(void){
    return drv8833_init();
}

bool traction_set_ref(float vel_a, float vel_b){
    stopped = 0;
    ref.left = vel_a;
    ref.right = vel_b; 
    return true;
}

bool traction_tick(void){
    if (stopped) return true;
    pid(MOTOR_A, ENC_A, ref.left);
    pid(MOTOR_B, ENC_B, ref.right);
    return true;
}

bool traction_stop(void){
    stopped = 1;
    ref.left = 0;
    ref.right = 0;
    drv8833_stop();
    //Tengo entendido que tambien se debería resetear la integral de pid
    return true;
}