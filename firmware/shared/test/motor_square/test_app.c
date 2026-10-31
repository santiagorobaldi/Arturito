/*
 * Test de motores: recorrido en "cuadrado" en lazo abierto (EDU-CIAA NXP)
 * APP_MODE=motor_square_test
 *
 * Secuencia: espera 3 s -> avanza 2 s -> gira a la derecha sobre su eje
 *            -> avanza 2 s -> gira a la derecha -> avanza 2 s -> frena.
 *
 * Ejercita: driver_drv8833 -> hal_pwm (EDU-CIAA) -> DRV8833 -> motores.
 * No usa encoders ni lazo PI: el giro de ~90 grados se logra por TIEMPO
 * (TURN_US) y hay que calibrarlo sobre el piso.
 *
 * Mensajes: se envian directamente por UART_USB (puerto COM del USB DEBUG,
 * 115200 8N1) con sAPI. No usa telemetry_app ni hal_telemetry.
 *
 * Es una maquina de estados NO bloqueante: app_update() se llama
 * repetidamente y consulta hal_time_us().
 *
 * Conexion (hal_pwm EDU-CIAA, modo BASIC):
 *   AIN1 <- PWM1 = T_COL2    AIN2 <- PWM2 = T_COL0    (motor A = izquierda)
 *   BIN1 <- PWM3 = T_FIL2    BIN2 <- PWM4 = T_FIL3    (motor B = derecha)
 */

#include "main.h"
#include "sapi.h"
#include "driver_drv8833.h"
#include "hal_time.h"
#include <stdint.h>
#include <stdbool.h>

/* ------------------------- Parametros a calibrar ------------------------- */

#define FWD_DUTY_PCT      95.0f      /* PWM de avance (0..100)                 */
#define TURN_DUTY_PCT     90.0f      /* PWM de cada rueda durante el giro      */

#define START_DELAY_US    3000000U   /* tiempo para soltar el rover            */
#define FWD_US            10000000U   /* cada tramo recto                       */
#define TURN_US           1200000U    /* giro de ~90 grados (CALIBRAR)          */
#define RAMP_US           200000U    /* rampa de arranque                      */
#define BRAKE_US          250000U    /* freno activo entre maniobras           */
#define SETTLE_US         300000U    /* motores libres antes de la siguiente   */
#define CONTROL_PERIOD_US 10000U     /* cada cuanto se reescribe el PWM        */

#define SEGMENTS          4U         /* tramos rectos (3 = lo pedido)          */

#define USB_BAUD          115200U

/* Signos de montaje (iguales a traction.c).
 * Convencion: duty positivo = la rueda empuja al rover hacia ADELANTE.
 * Si una rueda gira al reves, invertir SOLO su signo. */
#define MOTOR_LEFT_SIGN    ( -1.0f)
#define MOTOR_RIGHT_SIGN   ( -1.0f)

/* ------------------------------------------------------------------------- */

typedef enum {
    PH_WAIT,      /* espera inicial, motores libres */
    PH_FORWARD,   /* ambas ruedas adelante          */
    PH_TURN,      /* giro a la derecha en el lugar  */
    PH_BRAKE,     /* freno activo                   */
    PH_COAST,     /* motores libres                 */
    PH_DONE,      /* secuencia terminada            */
    PH_FAULT      /* error: motores detenidos       */
} phase_t;

static phase_t  phase;
static uint32_t phase_start_us;
static uint32_t last_control_us;
static unsigned segments_done;    /* tramos rectos completados           */
static bool     last_was_turn;    /* el ultimo movimiento fue un giro    */
static bool     motor_ready;

/* Envia una linea por el puerto serie del USB DEBUG. */
static void log_line(const char *line)
{
    uartWriteString(UART_USB, line);
    uartWriteString(UART_USB, "\r\n");
}

/* left_pct / right_pct: -100..100, positivo = rueda hacia adelante. */
static bool drive(float left_pct, float right_pct)
{
    bool ok = drv8833_set_pwm(MOTOR_A, MOTOR_LEFT_SIGN  * left_pct);
    /* Intentar ambos motores aunque falle el primero. */
    if (!drv8833_set_pwm(MOTOR_B, MOTOR_RIGHT_SIGN * right_pct)) ok = false;
    return ok;
}

static void fault(const char *reason)
{
    if (motor_ready && !drive(0.0f, 0.0f))
        log_line("ERROR al cortar PWM: desconectar alimentacion de los motores.");
    phase = PH_FAULT;
    log_line(reason);
}

static void enter_phase(phase_t next, uint32_t now)
{
    phase = next;
    phase_start_us = now;
    last_control_us = now - CONTROL_PERIOD_US;   /* fuerza escritura inmediata */

    switch (next) {
    case PH_FORWARD:
        log_line("AVANCE");
        break;
    case PH_TURN:
        log_line("GIRO A LA DERECHA");
        break;
    case PH_BRAKE:
        if (!drv8833_brake()) { fault("ERROR al aplicar freno; motores detenidos."); return; }
        break;
    case PH_COAST:
        if (!drive(0.0f, 0.0f)) { fault("ERROR al soltar motores."); return; }
        break;
    case PH_DONE:
        if (!drive(0.0f, 0.0f)) { fault("ERROR al soltar motores."); return; }
        log_line("=== SECUENCIA TERMINADA. Reiniciar para repetir. ===");
        break;
    default:
        break;
    }
}

/* Duty con rampa lineal de arranque durante RAMP_US. */
static float ramped(float target_pct, uint32_t elapsed_us)
{
    if (elapsed_us >= RAMP_US) return target_pct;
    return target_pct * ((float)elapsed_us / (float)RAMP_US);
}

void app_init(void)
{
    motor_ready = false;
    phase = PH_FAULT;      /* hasta que la inicializacion termine bien */
    segments_done = 0U;
    last_was_turn = false;

    /* Configura el UART del USB DEBUG (idempotente si boardInit ya lo hizo). */
    uartConfig(UART_USB, USB_BAUD);

    log_line("=== TEST MOTORES: RECORRIDO EN CUADRADO (lazo abierto) ===");

    if (!hal_time_init() || !drv8833_init()) {
        log_line("ERROR inicializando reloj/DRV8833; no iniciar motores.");
        return;
    }
    motor_ready = true;

    if (!drive(0.0f, 0.0f)) {
        fault("ERROR al poner salidas a cero.");
        return;
    }

    log_line("En 3 s: avance 2 s, giro derecha, avance 2 s, giro derecha, avance 2 s.");
    enter_phase(PH_WAIT, hal_time_us());
}

void app_update(void)
{
    if (phase == PH_FAULT || phase == PH_DONE) return;

    const uint32_t now     = hal_time_us();
    const uint32_t elapsed = (uint32_t)(now - phase_start_us);

    switch (phase) {

    case PH_WAIT:
        if (elapsed >= START_DELAY_US) enter_phase(PH_FORWARD, now);
        break;

    case PH_FORWARD:
        if (elapsed >= FWD_US) {
            last_was_turn = false;
            enter_phase(PH_BRAKE, now);
        } else if ((uint32_t)(now - last_control_us) >= CONTROL_PERIOD_US) {
            const float d = ramped(FWD_DUTY_PCT, elapsed);
            if (!drive(d, d)) { fault("ERROR al aplicar PWM; motores detenidos."); return; }
            last_control_us = now;
        }
        break;

    case PH_TURN:
        if (elapsed >= TURN_US) {
            last_was_turn = true;
            enter_phase(PH_BRAKE, now);
        } else if ((uint32_t)(now - last_control_us) >= CONTROL_PERIOD_US) {
            const float d = ramped(TURN_DUTY_PCT, elapsed);
            /* Izquierda adelante, derecha atras. */
            if (!drive(d, -d)) { fault("ERROR al aplicar PWM; motores detenidos."); return; }
            last_control_us = now;
        }
        break;

    case PH_BRAKE:
        if (elapsed >= BRAKE_US) enter_phase(PH_COAST, now);
        break;

    case PH_COAST:
        if (elapsed >= SETTLE_US) {
            if (last_was_turn) {
                enter_phase(PH_FORWARD, now);
            } else {
                segments_done++;
                if (segments_done >= SEGMENTS) enter_phase(PH_DONE, now);
                else                           enter_phase(PH_TURN, now);
            }
        }
        break;

    default:
        break;
    }
}