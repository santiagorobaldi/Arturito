
#include "hal_time.h"

#include "sapi.h"
#include "chip.h"

#define TIMER_FREQUENCY_HZ 1000000UL

static bool initialized = false;

/* Configura TIMER0 como contador libre de microsegundos. */
bool hal_time_init(void)
{
    if (initialized) {
        return true;
    }

    /* Habilita el reloj del periférico TIMER0. */
    Chip_TIMER_Init(LPC_TIMER0);

    /* Obtiene la frecuencia real configurada para el timer. */
    uint32_t clock_hz =
        Chip_Clock_GetRate(CLK_MX_TIMER0);

    /* Solo aceptamos un divisor entero para obtener 1 MHz. */
    if (clock_hz < TIMER_FREQUENCY_HZ ||
        (clock_hz % TIMER_FREQUENCY_HZ) != 0U) {
        return false;
    }

    uint32_t divisor = clock_hz / TIMER_FREQUENCY_HZ;

    /* Detiene el timer antes de modificar su configuración. */
    Chip_TIMER_Disable(LPC_TIMER0);

    /* Modo temporizador, sin capturas ni interrupciones. */
    LPC_TIMER0->CTCR = 0U;
    LPC_TIMER0->CCR = 0U;
    LPC_TIMER0->MCR = 0U;

    /* Un incremento de TC por cada microsegundo. */
    Chip_TIMER_PrescaleSet(LPC_TIMER0, divisor - 1U);

    /* Reinicia el contador y comienza a contar. */
    Chip_TIMER_Reset(LPC_TIMER0);
    Chip_TIMER_Enable(LPC_TIMER0);

    initialized = true;
    return true;
}

/* Lee el contador de hardware configurado a 1 MHz. */
uint32_t hal_time_us(void)
{
    if (!initialized) {
        return 0U;
    }

    return Chip_TIMER_ReadCount(LPC_TIMER0);
}

/* Calcula diferencias mediante aritmética sin signo. */
uint32_t hal_time_elapsed_us(uint32_t start_us)
{
    return hal_time_us() - start_us;
}

/* Funciones originales de sAPI, conservadas por compatibilidad. */
void hal_delay(uint32_t ms)
{
    delay(ms);
}

uint32_t hal_millis(void)
{
    return tickRead();
}

uint32_t hal_micros(void)
{
    /* Esta API antigua conserva su resolución real de 1 ms. */
    return tickRead() * 1000U;
}
