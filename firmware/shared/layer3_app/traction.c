#include "traction.h"
#include "driver_drv8833.h"

//#include "driver_as5600.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct{
    float izq;
    float der;
}motor_vel_t;

static motor_vel_t ref = {0,0};

static bool pid(drv8833_t m, as5600_t n, float vel){  
    //conversión de vel -> pwm
    int pwm = (vel > 100) ? 100 : vel;
    return drv8833_set_pwm(m,pwm);
}

bool traction_init(void){
    return drv8833_init();
}

bool traction_set_ref(float vel_a, float vel_b){
    ref.izq = vel_a;
    ref.der = vel_b; 
    return true;
}

bool traction_tick(void){
    pid(MOTOR_A, ENC_A, ref.izq);
    pid(MOTOR_B, ENC_B, ref.der);
    return true;
}

bool traction_stop(void){
    ref.izq = 0;
    ref.der = 0;
    drv8833_stop();
    //Tengo entendido que tambien se debería resetear la integral de pid
    return true;
}