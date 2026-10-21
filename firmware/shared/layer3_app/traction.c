#include "traction.h"
#include "driver_drv8833.h"

#include <stdint.h>
#include <stdbool.h>

bool traction_init(void){
    return drv8833_init();
}

bool traction_start_a_b(int pwm_a, int pwm_b){
    drv8833_start(A,pwm_a);
    drv8833_start(B,pwm_b);
    return true;
}

bool traction_stop(void){
    drv8833_stop(A);
    drv8833_stop(B);
    return true;
}