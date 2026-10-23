// edu_ciaa/hal_pwm.c
// Implementación de hal_pwm.h para EDU-CIAA-NXP (LPC4337) usando sAPI (firmware_v3).
// Revisado contra api_reference_es.md (sección PWM):
//   - Se configura con pwmInit(), no pwmConfig().
//   - pwmWrite(pwmNumber, uint8_t percent): el duty es un ENTERO en PORCENTAJE (0..100),
//     o sea resolución de 1 %. No es 0..255.
//   - pwmWrite() retorna siempre TRUE según la referencia: no sirve para detectar errores.
//   - La referencia NO documenta frecuencia ni qué pin  corresponde a cada PWMn.
//
// A VERIFICAR antes de usar:
//  1) Frecuencia real del PWM de sAPI (ver sapi_pwm.c) y ajustar SAPI_PWM_FREQ_HZ.
//  2) Qué pin físico sale por cada PWMn y cuáles están libres en tu cableado.

#include "hal_pwm.h"
#include "sapi.h"
#include "sapi_pwm.h"
#include "sapi_sct.h" //

// NO está en la referencia. Verificar en sapi_pwm.c.
#define SAPI_PWM_FREQ_HZ   1000U


// ÚNICA tabla con las salidas PWM de la EDU-CIAA.
// Los PWMn son PLACEHOLDERS: elegilos según pinout y pines libres.
static const pwmMap_t hw[HAL_PWM_CH_COUNT] = {
    [HAL_PWM_CH_1] = PWM1,
    [HAL_PWM_CH_2] = PWM2,
    [HAL_PWM_CH_3] = PWM3,
    [HAL_PWM_CH_4] = PWM4,
};

static bool sct_ready = false;
static bool ch_ready[HAL_PWM_CH_COUNT] = { false };

bool hal_pwm_init(hal_pwm_pin_t ch, uint32_t freq_hz)
{
    if ((unsigned)ch >= HAL_PWM_CH_COUNT) return false;

    // sAPI no permite elegir frecuencia: solo aceptamos la que ya usa.
    if (freq_hz != SAPI_PWM_FREQ_HZ) return false;

    // Primera vez: habilita el/los timers (SCT) en modo PWM.
    if (!sct_ready) {
        if (!pwmInit(0, PWM_ENABLE)) return false;
        sct_ready = true;
    }

    // Habilita la salida de este canal (se hace por cada salida PWM).
    if (!pwmInit(hw[ch], PWM_ENABLE_OUTPUT)) return false;
    pwmWrite(hw[ch], 0);                       // arranca apagado

    ch_ready[ch] = true;
    return true;
}

bool hal_pwm_set_duty(hal_pwm_pin_t ch, float duty_pct)
{
    if ((unsigned)ch >= HAL_PWM_CH_COUNT || !ch_ready[ch]) return false;

    if (duty_pct < 0.0f)   duty_pct = 0.0f;
    if (duty_pct > 100.0f) duty_pct = 100.0f;

    // sAPI recibe porcentaje entero: se redondea al 1 % más cercano.
    const uint8_t percent = (uint8_t)(duty_pct + 0.5f);

    return pwmWrite(hw[ch], percent);          // retorna siempre TRUE (según la referencia)
}