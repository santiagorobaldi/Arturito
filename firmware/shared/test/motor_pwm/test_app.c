/* Ensayo del DRV8833 en lazo abierto: avance, retroceso y giro en el lugar. */
#include "main.h"
#include "driver_drv8833.h"
#include "telemetry_app.h"
#include "hal_time.h"

/* Polaridad de avance usada por traction.c y motor_encoder_test. */
#define LEFT_FORWARD_PCT    100.0f
#define RIGHT_FORWARD_PCT (-100.0f)

static const struct {
    float motor_a_pct;
    float motor_b_pct;
    uint32_t duration_us;
    const char *label;
} steps[] = {
    {0, 0, 3000000U, "Inicio en 3 s. Motores en cero."},
    { LEFT_FORWARD_PCT,  RIGHT_FORWARD_PCT, 1000000U, "Avance 100% (1 s)"},
    {-LEFT_FORWARD_PCT, -RIGHT_FORWARD_PCT, 1000000U, "Retroceso 100% (1 s)"},
    { LEFT_FORWARD_PCT, -RIGHT_FORWARD_PCT, 4000000U, "Giro sobre el eje a la derecha 100% (4 s)"}
};
#define STEP_COUNT (sizeof(steps) / sizeof(steps[0]))

static unsigned step = STEP_COUNT;
static uint32_t started_us;

static bool drive(float a, float b)
{
    bool ok_a = drv8833_set_pwm(MOTOR_A, a);
    bool ok_b = drv8833_set_pwm(MOTOR_B, b);
    return ok_a && ok_b;
}

static void enter_step(unsigned next)
{
    step = next;
    if (step == STEP_COUNT) {
        if (drive(0, 0))
            telemetry_app_send_line("Fin. Motores en cero. Reiniciar para repetir.");
        else
            telemetry_app_send_line("ERROR al cortar PWM. Desconectar alimentacion de motores.");
        return;
    }
    if (!drive(steps[step].motor_a_pct, steps[step].motor_b_pct)) {
        step = STEP_COUNT;
        if (drive(0, 0))
            telemetry_app_send_line("ERROR al aplicar PWM. Ensayo detenido, motores en cero.");
        else
            telemetry_app_send_line("ERROR al cortar PWM. Desconectar alimentacion de motores.");
        return;
    }
    started_us = hal_time_us();
    telemetry_app_send_line(steps[step].label);
}

void app_init(void)
{
    step = STEP_COUNT;
    if (!telemetry_app_init()) return;
    if (!hal_time_init() || !drv8833_init()) {
        telemetry_app_send_line("ERROR inicializando reloj o DRV8833.");
        return;
    }
    enter_step(0);
}

void app_update(void)
{
    if (step < STEP_COUNT && hal_time_elapsed_us(started_us) >= steps[step].duration_us)
        enter_step(step + 1);
}
