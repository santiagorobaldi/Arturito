#include "main.h"
#include "driver_drv8833.h"
#include "driver_encoder.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define TEST_DUTY_LEFT_PCT 100.0f
#define TEST_DUTY_RIGHT_PCT 100.0f
#define START_DELAY_US 3000000U
#define RUN_TIME_US 15000000U
#define OFF_TIME_US 5000000U
#define SAMPLE_PERIOD_US 5000U
#define MAX_SAMPLE_GAP_US 50000U
#define REPORT_PERIOD_US 250000U
#define PI_F 3.14159265359f
#define TWO_PI_F (2.0f * PI_F)
#define WHEEL_COUNT 2U
/* Ensayo actual: ambos motores y un AS5600 en I2C A. */
#define READ_RIGHT_ENCODER false
#define STATUS_PERIOD_US 1000000U

typedef struct {
    const char *name;
    drv8833_motor_t motor;
    encoder_t encoder;
    hal_i2c_bus_t bus;
    float duty;
    bool read_encoder;
} wheel_config_t;
static const wheel_config_t wheels[WHEEL_COUNT] = {
    {"IZQ", MOTOR_A, ENC_IZQ, HAL_I2C_BUS_A, TEST_DUTY_LEFT_PCT, true},
    {"DER", MOTOR_B, ENC_DER, HAL_I2C_BUS_B, TEST_DUTY_RIGHT_PCT, READ_RIGHT_ENCODER}
};
typedef struct {
    float angle;
    float window_angle;
    uint32_t sample_us;
    uint32_t window_us;
} wheel_sample_t;
static wheel_sample_t samples[WHEEL_COUNT];
static bool active, motor_ready, running;
static uint32_t wait_us, start_us, run_start_us, last_poll_us, report_us;
static char stopped_reason[160];
static uint32_t status_ms;

static bool set_motors(bool enable)
{
    bool ok = true;
    for (unsigned i = 0; i < WHEEL_COUNT; ++i) {
        /* Intentar ambos canales aunque falle el primero. */
        if (!drv8833_set_pwm(wheels[i].motor, enable ? wheels[i].duty : 0.0f))
            ok = false;
    }
    return ok;
}
static void stop_test(const char *reason)
{
    if (motor_ready && !set_motors(false))
        telemetry_app_send_line("ERROR al cortar PWM: desconectar alimentacion de los motores.");
    active = false;
    running = false;
    snprintf(stopped_reason, sizeof(stopped_reason), "%s", reason);
    status_ms = hal_millis();
    telemetry_app_send_line(stopped_reason);
}
static void encoder_error(unsigned i, const char *operation, encoder_status_t status)
{
    char message[140];
    snprintf(message, sizeof(message),
             "%s: error %s (%d); ambos motores detenidos. Reiniciar para reintentar.",
             wheels[i].name, operation, (int)status);
    stop_test(message);
}
void app_init(void)
{
    if (!telemetry_app_init()) {
        hal_telemetry_write("ERROR: no se pudo iniciar la telemetria.");
    }

    active = false;
    motor_ready = false;
    running = false;
    stopped_reason[0] = '\0';
    wait_us = START_DELAY_US;
    telemetry_app_send_line("=== TEST DOS MOTORES + ENCODER IZQ (I2C A) ===");
    if (READ_RIGHT_ENCODER)
        telemetry_app_send_line("Encoder DER (I2C B) tambien habilitado.");
    if (!hal_time_init() || !drv8833_init()) {
        stop_test("ERROR inicializando reloj/DRV8833; no iniciar motores.");
        return;
    }
    motor_ready = true;
    if (!set_motors(false)) {
        stop_test("ERROR al poner salidas a cero.");
        return;
    }
    for (unsigned i = 0; i < WHEEL_COUNT; ++i) {
        if (!wheels[i].read_encoder) continue;
        if (!hal_i2c_init(wheels[i].bus)) {
            encoder_error(i, "inicio I2C", ENCODER_ERROR_COMMUNICATION);
            return;
        }
        encoder_status_t status = encoder_init(wheels[i].encoder);
        if (status != ENCODER_OK) {
            encoder_error(i, "inicio encoder", status);
            return;
        }
        status = encoder_get_angle_rad(wheels[i].encoder, &samples[i].angle);
        if (status != ENCODER_OK || !isfinite(samples[i].angle)) {
            encoder_error(i, "primera lectura", status == ENCODER_OK ?
                          ENCODER_ERROR_INTERNAL_API : status);
            return;
        }
        samples[i].sample_us = hal_time_us();
        samples[i].window_angle = 0.0f;
        samples[i].window_us = 0U;
    }
    char message[140];
    snprintf(message, sizeof(message),
             "En 3 s: IZQ PWM=%d%%, DER PWM=%d%%; ciclo 15 s activos / 5 s detenidos.",
             (int)wheels[0].duty, (int)wheels[1].duty);
    telemetry_app_send_line(message);
    telemetry_app_send_line("RPM del eje de cada iman; signo segun montaje. Muestreo 5 ms, reporte 250 ms.");
    start_us = last_poll_us = report_us = hal_time_us();
    active = true;
}
void app_update(void)
{
    telemetry_app_update();

    if (!active) {
        const uint32_t now_ms = hal_millis();
        if (stopped_reason[0] != '\0' &&
            (uint32_t)(now_ms - status_ms) >= STATUS_PERIOD_US / 1000U) {
            telemetry_app_send_line(stopped_reason);
            status_ms = now_ms;
        }
        return;
    }
    uint32_t now = hal_time_us();
    if ((uint32_t)(now - last_poll_us) < SAMPLE_PERIOD_US) return;
    for (unsigned i = 0; i < WHEEL_COUNT; ++i) {
        if (!wheels[i].read_encoder) continue;
        float angle;
        encoder_status_t status = encoder_get_angle_rad(wheels[i].encoder, &angle);
        if (status != ENCODER_OK || !isfinite(angle)) {
            encoder_error(i, "lectura", status == ENCODER_OK ?
                          ENCODER_ERROR_INTERNAL_API : status);
            return;
        }
        const uint32_t read_us = hal_time_us();
        const uint32_t dt = (uint32_t)(read_us - samples[i].sample_us);
        if (dt > MAX_SAMPLE_GAP_US) {
            char message[140];
            snprintf(message, sizeof(message),
                     "%s: ERROR muestreo dt=%lu us, limite=%lu us; ambos motores detenidos.",
                     wheels[i].name, (unsigned long)dt, (unsigned long)MAX_SAMPLE_GAP_US);
            stop_test(message);
            return;
        }
        float delta = angle - samples[i].angle;
        /* Requiere menos de media vuelta entre lecturas de este encoder. */
        if (delta > PI_F) delta -= TWO_PI_F;
        else if (delta < -PI_F) delta += TWO_PI_F;
        samples[i].window_angle += delta;
        samples[i].window_us += dt;
        samples[i].angle = angle;
        samples[i].sample_us = read_us;
    }
    now = hal_time_us();
    last_poll_us = now;
    if (!running && (uint32_t)(now - start_us) >= wait_us) {
        if (!set_motors(true)) {
            stop_test("ERROR al aplicar PWM; ambos motores detenidos.");
            return;
        }
        running = true;
        run_start_us = now;
        telemetry_app_send_line("Ambos motores activos durante 15 s.");
    }
    if (running && (uint32_t)(now - run_start_us) >= RUN_TIME_US) {
        if (!set_motors(false)) {
            stop_test("ERROR al detener motores.");
            return;
        }
        running = false;
        start_us = now;
        wait_us = OFF_TIME_US;
        telemetry_app_send_line("Ambos PWM a 0% durante 5 s; luego arranque automatico.");
    }
    if ((uint32_t)(now - report_us) >= REPORT_PERIOD_US) {
        for (unsigned i = 0; i < WHEEL_COUNT; ++i) {
            if (!wheels[i].read_encoder) {
                char message[80];
                snprintf(message, sizeof(message), "%s | PWM=%d%% | encoder no habilitado",
                         wheels[i].name, running ? (int)wheels[i].duty : 0);
                telemetry_app_send_line(message);
                continue;
            }
            const float rpm = samples[i].window_angle * (60000000.0f / TWO_PI_F) /
                              (float)samples[i].window_us;
            char message[120];
            snprintf(message, sizeof(message), "%s | PWM=%d%% | angulo=%.2f deg | velocidad=%.2f RPM",
                     wheels[i].name, running ? (int)wheels[i].duty : 0,
                     (double)(samples[i].angle * (180.0f / PI_F)), (double)rpm);
            telemetry_app_send_line(message);
            samples[i].window_angle = 0.0f;
            samples[i].window_us = 0U;
        }
        report_us = now;
    }
}
