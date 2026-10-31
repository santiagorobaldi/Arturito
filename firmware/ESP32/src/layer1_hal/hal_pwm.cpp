#include <Arduino.h>
#include <esp_arduino_version.h>

extern "C" {
#include "hal_pwm.h"
}

#define PWM_RESOLUTION_BITS  10
#define PWM_DUTY_MAX         ((1U << PWM_RESOLUTION_BITS) - 1U)   // 1023

// Única tabla con los GPIO del PWM
static const uint8_t hw_pin[HAL_PWM_CH_COUNT] = { //a definir pinout de esp32
    [HAL_PWM_CH_1] = 26, 
    [HAL_PWM_CH_2] = 27,
    [HAL_PWM_CH_3] = 32,
    [HAL_PWM_CH_4] = 33,
};

static bool     ch_ready[HAL_PWM_CH_COUNT] = { false }; //seguimiento de canales activos/ocupados

static uint32_t shared_freq = 0;     //todos los pines deben compartir la frecuencia

extern "C" bool hal_pwm_init(hal_pwm_pin_t ch, uint32_t freq_hz)
{
    if ((unsigned)ch >= HAL_PWM_CH_COUNT || freq_hz == 0) return false; //guarda de si el canal existe y es una frecuencia aceptable

    if (shared_freq == 0)              shared_freq = freq_hz;
    else if (freq_hz != shared_freq)   return false;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
    if (!ledcAttach(hw_pin[ch], freq_hz, PWM_RESOLUTION_BITS)) return false;
    if (!ledcWrite(hw_pin[ch], 0)) return false;
#else
    if (ledcSetup((uint8_t)ch, freq_hz, PWM_RESOLUTION_BITS) == 0) return false;
    ledcAttachPin(hw_pin[ch], (uint8_t)ch);
    ledcWrite((uint8_t)ch, 0);
#endif

    ch_ready[ch] = true; 
    return true;
}

extern "C" bool hal_pwm_set_duty(hal_pwm_pin_t ch, float duty_pct)
{
    if ((unsigned)ch >= HAL_PWM_CH_COUNT || !ch_ready[ch]) return false; //guarda de si el canal exsiste y está ready

    if (duty_pct < 0.0f)   duty_pct = 0.0f;
    if (duty_pct > 100.0f) duty_pct = 100.0f;

    const uint32_t duty = (uint32_t)(duty_pct * (float)PWM_DUTY_MAX / 100.0f + 0.5f);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    return ledcWrite(hw_pin[ch], duty);
#else
    ledcWrite((uint8_t)ch, duty);
    return true;
#endif
}
