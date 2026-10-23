#include "driver_drv8833.h"
#include "hal_pwm.h"
#include "hal_gpio.h"

#define PWM_FREQ_HZ  20000


typedef enum {//Puertos internos del componente
    DRV_AIN1 = 0,
    DRV_AIN2,
    DRV_BIN1,
    DRV_BIN2,
    DRV_PWM_COUNT
} drv_pwm_role_t;

// mapeo de puertos del driver a canales de hal_pwm
static const hal_pwm_pin_t role_to_hal[DRV_PWM_COUNT] = {
    [DRV_AIN1] = HAL_PWM_CH_1,
    [DRV_AIN2] = HAL_PWM_CH_2,
    [DRV_BIN1] = HAL_PWM_CH_3,
    [DRV_BIN2] = HAL_PWM_CH_4,
}; 

// puertos de un motor
typedef struct {
    drv_pwm_role_t in1;
    drv_pwm_role_t in2;
} motor_roles_t;

// vector de puertos de motores
static const motor_roles_t motor_roles[MOTOR_COUNT] = {
    [MOTOR_A] = { DRV_AIN1, DRV_AIN2 },
    [MOTOR_B] = { DRV_BIN1, DRV_BIN2 },
};

bool drv8833_init(void) {
    if (!hal_gpio_init(HAL_GPIO_DRV_SLEEP, HAL_GPIO_OUT))       return false;
    if (!hal_gpio_init(HAL_GPIO_DRV_FAULT, HAL_GPIO_IN_PULLUP)) return false;

    for (int i = 0; i < DRV_PWM_COUNT; i++) {
        if (!hal_pwm_init(role_to_hal[i], PWM_FREQ_HZ)) return false;
    }
    return hal_gpio_write(HAL_GPIO_DRV_SLEEP, true);   // chip activo
}

bool drv8833_set_pwm(drv8833_motor_t m, int pwm) {
    if ((unsigned)m >= MOTOR_COUNT) return false;

    if (pwm >  MAX_PWM) pwm =  MAX_PWM;
    if (pwm < -MAX_PWM) pwm = -MAX_PWM;

    int abs_pwm = (pwm >= 0) ? pwm : -pwm;
    float duty = (float)abs_pwm * 100.0f / (float)MAX_PWM;

    hal_pwm_pin_t ch1 = role_to_hal[motor_roles[m].in1];
    hal_pwm_pin_t ch2 = role_to_hal[motor_roles[m].in2];

    bool ok;
    if (pwm >= 0) {
        ok &= hal_pwm_set_duty(ch2, 0.0f); 
        ok  = hal_pwm_set_duty(ch1, duty);//Si de reversa a adelante se ejecutara esta lineas primero, el motor activa los breaks. Por eso primero seteamos en cero el otro ch
    } else {
        ok  = hal_pwm_set_duty(ch1, 0.0f);
        ok &= hal_pwm_set_duty(ch2, duty);
    }
    return ok;
}

bool drv8833_stop(void) {
    bool ok = true;
    for (int i = 0; i < DRV_PWM_COUNT; i++) {
        ok &= hal_pwm_set_duty(role_to_hal[i], 0.0f);
    }
    return ok;
}

bool drv8833_fault(void) {
    bool level = true;
    if (!hal_gpio_read(HAL_GPIO_DRV_FAULT, &level)) return true;  // si no puedo leer, asumo falla
    return !level;   // nFAULT activo en bajo
}