// edu_ciaa/hal_pwm.c
// Implementación de hal_pwm.h para EDU-CIAA-NXP (LPC4337) usando sAPI (firmware_v3).
//
// Dos modos, elegidos en compilación:
//
//   BASIC definido   -> usa solo la API pública de sAPI: pwmInit(0, PWM_ENABLE).
//                       Frecuencia FIJA en 1 kHz (PWM_FREC en sapi_pwm.c).
//                       El parámetro freq_hz de hal_pwm_init se IGNORA.
//                       Es la ruta más conservadora: es el uso documentado de sAPI.
//
//   BASIC comentado  -> llama directo a Sct_Init(freq_hz) y acepta la frecuencia pedida
//                       (por ejemplo 20 kHz). Todos los canales comparten frecuencia.
//                       PENDIENTE DE PROBAR en hardware.
//
// Comunes a ambos modos (verificado en sapi_pwm.c y sapi_sct.c):
//   - Duty: pwmWrite(pwmN, 0..255), ticks = ticks_por_ciclo * valor / 255. 256 pasos.
//     (La api_reference_es.md dice "porcentaje": es incorrecto.)
//   - pwmWrite() devuelve FALSE si el canal no fue habilitado.
//   - pwmInit(pwmN, PWM_ENABLE_OUTPUT) devuelve FALSE si el canal ya estaba habilitado.
//
// Mapeo PWMn -> CTOUT -> pin (pwmMap[] en sapi_pwm.c, SCTdataList[] en sapi_sct.c):
//   PWM0=CTOUT1 =P4_1 (T_FIL1)   PWM1=CTOUT12=P7_5 (T_COL2)   PWM2=CTOUT10=P1_5 (T_COL0)
//   PWM3=CTOUT0 =P4_2 (T_FIL2)   PWM4=CTOUT3 =P4_3 (T_FIL3)   PWM5=CTOUT13=P7_4 (T_COL1)
//   PWM6=CTOUT7 =P6_12 (GPIO8)   PWM10=CTOUT6=P6_5 (GPIO2)
//   PWM7..PWM9 = LED1..LED3 de la placa (no usar para motores)
//
// IMPORTANTE: nadie más debe llamar pwmInit(x, PWM_ENABLE) una vez iniciado el HAL:
//             reinicia el SCT.

#include "hal_pwm.h"
#include "sapi.h"
#include "sapi_pwm.h"
#include "sapi_sct.h"

// ---------------------------------------------------------------------------
// Selector de modo: comentar la línea siguiente para usar frecuencia configurable.
// (También se puede definir desde el build con -DBASIC.)
// ---------------------------------------------------------------------------
#define BASIC

#define SAPI_PWM_VALUE_MAX   255U     // pwmWrite recibe 0..255

#ifdef BASIC
#define BASIC_FREQ_HZ        1000U    // PWM_FREC de sapi_pwm.c
#else
// Mínimo de ticks por ciclo para no perder los 256 pasos de duty.
#define MIN_TICKS_PER_CYCLE  256U
#endif

// ÚNICA tabla con las salidas PWM de la EDU-CIAA.
static const pwmMap_t hw[HAL_PWM_CH_COUNT] = {
    [HAL_PWM_CH_1] = PWM1,   // T_COL2
    [HAL_PWM_CH_2] = PWM2,   // T_COL0
    [HAL_PWM_CH_3] = PWM3,   // T_FIL2
    [HAL_PWM_CH_4] = PWM4,   // T_FIL3
};

static bool     sct_ready   = false;
static uint32_t shared_freq = 0;
static bool     ch_ready[HAL_PWM_CH_COUNT] = { false };

bool hal_pwm_init(hal_pwm_pin_t ch, uint32_t freq_hz)
{
    if ((unsigned)ch >= HAL_PWM_CH_COUNT || freq_hz == 0) return false;
    if (ch_ready[ch]) return true;                   // ya inicializado (idempotente)

    if (!sct_ready) {
#ifdef BASIC
        // sAPI inicializa el SCT a 1 kHz. freq_hz se ignora.
        if (!pwmInit(0, PWM_ENABLE)) return false;
        shared_freq = BASIC_FREQ_HZ;
#else
        // Debe ir ANTES de habilitar las salidas: reinicia el SCT.
        Sct_Init(freq_hz);

        // Si la frecuencia es tan alta que quedan pocos ticks, el duty pierde resolución.
        if (Chip_SCTPWM_GetTicksPerCycle(LPC_SCT) < MIN_TICKS_PER_CYCLE) return false;

        shared_freq = freq_hz;
#endif
        sct_ready = true;
    }
#ifndef BASIC
    else if (freq_hz != shared_freq) {
        return false;                                // frecuencia única para todos los canales
    }
#endif

    if (!pwmInit(hw[ch], PWM_ENABLE_OUTPUT)) return false;
    if (!pwmWrite(hw[ch], 0)) return false;          // arranca apagado

    ch_ready[ch] = true;
    return true;
}

bool hal_pwm_set_duty(hal_pwm_pin_t ch, float duty_pct)
{
    if ((unsigned)ch >= HAL_PWM_CH_COUNT || !ch_ready[ch]) return false;

    if (duty_pct < 0.0f)   duty_pct = 0.0f;
    if (duty_pct > 100.0f) duty_pct = 100.0f;

    const uint8_t value = (uint8_t)(duty_pct * (float)SAPI_PWM_VALUE_MAX / 100.0f + 0.5f);
    return pwmWrite(hw[ch], value);
}