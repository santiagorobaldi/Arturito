
#include "hal_time.h"

#include <Arduino.h>
#include <esp_timer.h>

/* Arduino ya inicializa el temporizador de ESP-IDF. */
bool hal_time_init(void)
{
    return true;
}

/* Devuelve los microsegundos transcurridos desde el arranque. */
uint32_t hal_time_us(void)
{
    return (uint32_t)esp_timer_get_time();
}

/* Calcula una diferencia temporal teniendo en cuenta el desbordamiento. */
uint32_t hal_time_elapsed_us(uint32_t start_us)
{
    return hal_time_us() - start_us;
}

/* Funciones originales, conservadas sin cambios de comportamiento. */
void hal_delay(uint32_t ms)
{
    delay(ms);
}

uint32_t hal_millis(void)
{
    return millis();
}

uint32_t hal_micros(void)
{
    return micros();
}
