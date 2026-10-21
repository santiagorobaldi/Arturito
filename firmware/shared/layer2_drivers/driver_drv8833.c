#include "config.h"
#include "driver_drv8833.h"
#include "hal_pwm.h"

typedef struct{
    int in1_pin;
    int in2_pin;
} motor_pins_t;


static const motor_pins_t motor_pins [MOTOR_COUNT] = {
    [MOTOR_A] = {MOTOR_A_IN1_PIN, MOTOR_A_IN2_PIN}
    [MOTOR_B] = {MOTOR_B_IN1_PIN, MOTOR_B_IN2_PIN}
};


bool drv8833_init(){}

bool drv8833_set_pwm(drv8833_t m , int pwm){
    if (m>=MOTOR_COUNT) return false;

    const motor_pins_t *p = &motor_pins[m];

    if (pwm >=0){
        hal_pwm_set_duty(p->in1_pin, pwm);
        hal_pwm_set_duty(p->in2_pin,0);
    } else{
        hal_pwm_set_duty(p->in1_pin, 0);
        hal_pwm_set_duty(p->in2_pin,-pwm);
    }
    return true;
}

bool drv8833_stop(drv8833_t m){
    hal_pwm_set_duty(MOTOR_A_IN1_PIN, 0)
    hal_pwm_set_duty(MOTOR_A_IN2_PIN, 0)
    hal_pwm_set_duty(MOTOR_B_IN1_PIN, 0)
    hal_pwm_set_duty(MOTOR_B_IN2_PIN, 0)
}