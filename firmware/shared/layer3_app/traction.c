#include "traction.h"
#include "driver_drv8833.h"

//#include "driver_as5600.h"

#include <stdint.h>
#include <stdbool.h>

static float ref_izq=0, ref_der=0;

static bool pid(drv8833_t m, as5600_t n, int vel){  
    return drv8833_set_pwm(m,vel);
}

bool traction_init(void){
    return drv8833_init();
}

bool traction_set_ref(int vel_a, int vel_b){
    ref_izq = vel_a;
    ref_der = vel_b; 
}

bool traction_tick(){
    pid(MOTOR_A, ENC_A, ref_izq);
    pid(MOTOR_B, ENC_B, ref_der);
    return true;
}

bool traction_stop(void){
    ref_izq = 0;
    ref_der = 0;
    return true;
}