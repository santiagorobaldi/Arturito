/*
 * Test de motores + encoders con ciclo unico (EDU-CIAA NXP)
 * APP_MODE=motor_encoder_test
 *
 * Ciclo (una sola vez), con escalones de PWM 75-80-85-90-95 % (1 s cada uno):
 *   3 s espera
 *   5 s AMBOS ADELANTE          -> 2 s pausa
 *   5 s AMBOS ATRAS             -> 2 s pausa
 *   5 s IZQ ADELANTE / DER ATRAS -> 2 s pausa
 *   5 s IZQ ATRAS / DER ADELANTE -> 2 s pausa final y fin
 *
 * El encoder IZQ (I2C A) se lee durante TODO el ciclo, incluidas esperas y pausas.
 * El encoder DER (I2C B) no se inicializa ni se lee; su motor sigue todo el ciclo.
 *
 * Salida (misma linea por dos caminos):
 *   - telemetry_app_send_line() -> BOARD_TELEMETRY_UART (UART hacia el ESP/WiFi)
 *   - hal_telemetry_write()     -> UART_USB (monitor serie del USB DEBUG)
 *   Si BOARD_TELEMETRY_UART es UART_USB, solo se envia una copia (la primera) y
 *   se avisa que no hay salida hacia el ESP.
 *
 * Formato del reporte (cada 250 ms):
 *   t=12.250 IZQ(A):ADE 80% ang=210.5 v=45.2rpm | DER:ADE 80% encoder deshabilitado
 *   ADE/ATR/STOP = sentido ordenado a la RUEDA, % = PWM ordenado,
 *   ang = angulo del encoder en grados, v = RPM del eje del iman (signo segun montaje).
 *
 * Conexionado (segun hal_pwm, hal_i2c_educiaa.c y educiaa_board_config.h):
 *   Motores (DRV8833): AIN1<-T_COL2 (P7_5), AIN2<-T_COL0 (P1_5)  motor A = IZQ
 *                      BIN1<-T_FIL2 (P4_2), BIN2<-T_FIL3 (P4_3)  motor B = DER
 *   Encoder IZQ (bus A): I2C0, pines dedicados SDA y SCL de la EDU-CIAA
 *   Encoder DER (bus B): no requerido para este ensayo.
 *   UART al ESP32: UART_GPIO (UART0), GPIO1 = TX, GPIO2 = RX, 115200, 3,3 V
 *     EDU GPIO1(TX) -> ESP32 GPIO16 (RX2)   EDU GPIO2(RX) <- ESP32 GPIO17 (TX2)
 *     GND comun. UART_232 NO se puede usar: comparte P2_3/P2_4 con el bus B.
 */

#include "main.h"
#include "sapi.h"
#include "educiaa_board_config.h"
#include "driver_drv8833.h"
#include "driver_encoder.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* ------------------------- Parametros del ensayo ------------------------- */

#define START_DELAY_US     3000000U
#define RUN_US             5000000U      /* duracion de cada movimiento       */
#define PAUSE_US           2000000U      /* pausa entre movimientos           */

#define PWM_STEP_COUNT     5U            /* escalones dentro de cada movimiento */
static const int pwm_steps[PWM_STEP_COUNT] = { 75, 80, 85, 90, 95 };
#define STEP_US            (RUN_US / PWM_STEP_COUNT)

#define SAMPLE_PERIOD_US   5000U         /* lectura de encoders               */
#define MAX_SAMPLE_GAP_US  50000U
#define REPORT_PERIOD_US   250000U       /* reporte por telemetria            */

#define USB_BAUD           115200U

/* Signos de montaje de motores (iguales a traction.c).
 * Convencion: PWM positivo en la rueda = la rueda empuja al rover hacia ADELANTE.
 * Si una rueda gira al reves, invertir SOLO su signo. */
#define MOTOR_LEFT_SIGN    ( 1.0f)
#define MOTOR_RIGHT_SIGN   (-1.0f)

#define PI_F               3.14159265359f
#define TWO_PI_F           (2.0f * PI_F)
#define WHEEL_COUNT        2U
/* Solo la primera rueda de wheels[] tiene encoder conectado (IZQ, I2C A). */
#define ENCODER_COUNT      1U

/* ------------------------------------------------------------------------- */

typedef struct {
    const char *name;
    const char *bus_label;
    drv8833_motor_t motor;
    encoder_t encoder;
    hal_i2c_bus_t bus;
    float motor_sign;
} wheel_config_t;

static const wheel_config_t wheels[WHEEL_COUNT] = {
    {"IZQ", "A", MOTOR_A, ENC_IZQ, HAL_I2C_BUS_A, MOTOR_LEFT_SIGN},
    {"DER", "B", MOTOR_B, ENC_DER, HAL_I2C_BUS_B, MOTOR_RIGHT_SIGN}
};

typedef struct {
    const char *name;
    int8_t left_dir;      /* +1 adelante, -1 atras, 0 detenida */
    int8_t right_dir;
    bool run;             /* true = aplica escalones de PWM    */
    uint32_t duration_us;
} step_t;

static const step_t sequence[] = {
    { "ESPERA INICIAL",             0,  0, false, START_DELAY_US },
    { "AMBOS ADELANTE",             1,  1, true,  RUN_US },
    { "PAUSA",                      0,  0, false, PAUSE_US },
    { "AMBOS ATRAS",               -1, -1, true,  RUN_US },
    { "PAUSA",                      0,  0, false, PAUSE_US },
    { "IZQ ADELANTE / DER ATRAS",   1, -1, true,  RUN_US },
    { "PAUSA",                      0,  0, false, PAUSE_US },
    { "IZQ ATRAS / DER ADELANTE",  -1,  1, true,  RUN_US },
    { "PAUSA FINAL",                0,  0, false, PAUSE_US }
};
#define SEQ_COUNT ((unsigned)(sizeof(sequence) / sizeof(sequence[0])))

typedef struct {
    float angle;
    float window_angle;
    uint32_t sample_us;
    uint32_t window_us;
} wheel_sample_t;

static wheel_sample_t samples[ENCODER_COUNT];
static int  cmd_pct[WHEEL_COUNT];     /* PWM ordenado, con signo, convencion de rueda */
static bool active, motor_ready;
static unsigned step;
static uint32_t step_start_us, t0_us, last_poll_us, report_us;

/* Una linea por los dos caminos (sin duplicar si la telemetria ya es el USB). */
static void emit(const char *line)
{
    telemetry_app_send_line(line);
    if (BOARD_TELEMETRY_UART != UART_USB) {
        hal_telemetry_write(line);
    }
}

static bool apply_commands(void)
{
    bool ok = true;
    for (unsigned i = 0; i < WHEEL_COUNT; ++i) {
        /* Intentar ambos canales aunque falle el primero. */
        if (!drv8833_set_pwm(wheels[i].motor, wheels[i].motor_sign * (float)cmd_pct[i]))
            ok = false;
    }
    return ok;
}

static void stop_test(const char *reason)
{
    cmd_pct[0] = 0;
    cmd_pct[1] = 0;
    if (motor_ready && !apply_commands())
        emit("ERROR al cortar PWM: desconectar alimentacion de los motores.");
    active = false;
    emit(reason);
}

static void encoder_error(unsigned i, const char *operation, encoder_status_t status)
{
    char message[140];
    snprintf(message, sizeof(message),
             "%s: error %s (%d); ambos motores detenidos. Reiniciar para reintentar.",
             wheels[i].name, operation, (int)status);
    stop_test(message);
}

static void announce_step(void)
{
    char message[80];
    snprintf(message, sizeof(message), "FASE %u/%u: %s",
             step + 1U, SEQ_COUNT, sequence[step].name);
    emit(message);
}

static const char *dir_name(int pct)
{
    return (pct > 0) ? "ADE" : ((pct < 0) ? "ATR" : "STOP");
}

static int pct_abs(int pct)
{
    return (pct < 0) ? -pct : pct;
}

/* Avanza el ciclo y ordena los PWM. Devuelve false si hubo error. */
static bool update_sequence(uint32_t now)
{
    uint32_t elapsed = (uint32_t)(now - step_start_us);

    if (elapsed >= sequence[step].duration_us) {
        step++;
        if (step >= SEQ_COUNT) {
            cmd_pct[0] = 0;
            cmd_pct[1] = 0;
            if (!apply_commands()) {
                stop_test("ERROR al detener motores.");
                return false;
            }
            active = false;
            emit("=== CICLO TERMINADO. Reiniciar para repetir. ===");
            return true;
        }
        step_start_us = now;
        elapsed = 0U;
        announce_step();
    }

    const step_t *s = &sequence[step];
    int magnitude = 0;
    if (s->run) {
        unsigned idx = elapsed / STEP_US;
        if (idx >= PWM_STEP_COUNT) idx = PWM_STEP_COUNT - 1U;
        magnitude = pwm_steps[idx];
    }

    const int new_left  = s->left_dir  * magnitude;
    const int new_right = s->right_dir * magnitude;
    if (new_left != cmd_pct[0] || new_right != cmd_pct[1]) {
        cmd_pct[0] = new_left;
        cmd_pct[1] = new_right;
        if (!apply_commands()) {
            stop_test("ERROR al aplicar PWM; ambos motores detenidos.");
            return false;
        }
    }
    return true;
}

static void report(uint32_t now)
{
    float rpm[ENCODER_COUNT];
    for (unsigned i = 0; i < ENCODER_COUNT; ++i) {
        rpm[i] = (samples[i].window_us > 0U)
               ? samples[i].window_angle * (60000000.0f / TWO_PI_F) / (float)samples[i].window_us
               : 0.0f;
    }

    const uint32_t t_ms = (uint32_t)(now - t0_us) / 1000U;
    const float rad2deg = 180.0f / PI_F;
    char message[160];
    snprintf(message, sizeof(message),
             "t=%lu.%03lu %s(%s):%s %d%% ang=%.1f v=%.1frpm | %s:%s %d%% encoder deshabilitado",
             (unsigned long)(t_ms / 1000U), (unsigned long)(t_ms % 1000U),
             wheels[0].name, wheels[0].bus_label, dir_name(cmd_pct[0]), pct_abs(cmd_pct[0]),
             (double)(samples[0].angle * rad2deg), (double)rpm[0],
             wheels[1].name, dir_name(cmd_pct[1]), pct_abs(cmd_pct[1]));
    emit(message);

    for (unsigned i = 0; i < ENCODER_COUNT; ++i) {
        samples[i].window_angle = 0.0f;
        samples[i].window_us = 0U;
    }
}

void app_init(void)
{
    /* El USB se configura primero: si BOARD_TELEMETRY_UART es UART_USB, la
     * telemetria vuelve a configurarlo con su propia velocidad. */
    uartConfig(UART_USB, USB_BAUD);
    if (!telemetry_app_init()) {
        hal_telemetry_write("ERROR: no se pudo iniciar la telemetria.");
    }

    active = false;
    motor_ready = false;
    step = 0U;
    cmd_pct[0] = 0;
    cmd_pct[1] = 0;

    emit("=== TEST MOTORES + ENCODERS: CICLO UNICO ===");
    emit("Encoder habilitado: IZQ (I2C A). Encoder DER deshabilitado; ambos motores activos en el ensayo.");
    if (BOARD_TELEMETRY_UART == UART_USB) {
        emit("AVISO: BOARD_TELEMETRY_UART es UART_USB; no hay salida hacia el ESP.");
    }

    if (!hal_time_init() || !drv8833_init()) {
        emit("ERROR inicializando reloj/DRV8833; no iniciar motores.");
        return;
    }
    motor_ready = true;
    if (!apply_commands()) {
        stop_test("ERROR al poner salidas a cero.");
        return;
    }

    for (unsigned i = 0; i < ENCODER_COUNT; ++i) {
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

    emit("Escalones de PWM: 75-80-85-90-95 %, 1 s cada uno. ang en grados, v en RPM.");
    t0_us = step_start_us = last_poll_us = report_us = hal_time_us();
    active = true;
    announce_step();
}

void app_update(void)
{
    telemetry_app_update();

    if (!active) return;

    uint32_t now = hal_time_us();
    if (!update_sequence(now) || !active) return;

    if ((uint32_t)(now - last_poll_us) < SAMPLE_PERIOD_US) return;

    for (unsigned i = 0; i < ENCODER_COUNT; ++i) {
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
    if ((uint32_t)(now - report_us) >= REPORT_PERIOD_US) {
        report(now);
        report_us = now;
    }
}
