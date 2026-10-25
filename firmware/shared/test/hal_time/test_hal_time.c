
#include "test_hal_time.h"

#include "hal_time.h"
#include "hal_telemetry.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define TEST_PERIOD_US 2000000UL
#define RESOLUTION_ITERATIONS 10000U

static bool clock_ready = false;
static bool first_run = true;
static uint32_t last_run_us = 0U;

/* Mide una espera utilizando el reloj de alta resolución. */
static void test_delay(uint32_t delay_ms)
{
    char message[120];

    uint32_t start_ms = hal_millis();
    uint32_t start_us = hal_time_us();

    hal_delay(delay_ms);

    uint32_t elapsed_us =
        hal_time_elapsed_us(start_us);

    uint32_t elapsed_ms =
        hal_millis() - start_ms;

    snprintf(
        message,
        sizeof(message),
        "Espera: %lu ms | Reloj: %lu us | Millis: %lu ms",
        (unsigned long)delay_ms,
        (unsigned long)elapsed_us,
        (unsigned long)elapsed_ms
    );

    hal_telemetry_write(message);
}

/* Busca el menor incremento observable del reloj. */
static void test_resolution(void)
{
    char message[100];

    uint32_t previous = hal_time_us();
    uint32_t min_step = UINT32_MAX;

    for (uint32_t i = 0U;
         i < RESOLUTION_ITERATIONS;
         ++i) {

        uint32_t current = hal_time_us();
        uint32_t delta = current - previous;

        if (delta > 0U && delta < min_step) {
            min_step = delta;
        }

        previous = current;
    }

    if (min_step == UINT32_MAX) {
        hal_telemetry_write(
            "ERROR: no se observo avance del reloj."
        );
        return;
    }

    snprintf(
        message,
        sizeof(message),
        "Menor incremento observado: %lu us",
        (unsigned long)min_step
    );

    hal_telemetry_write(message);

    if (min_step == 1U) {
        hal_telemetry_write(
            "OK: se observaron incrementos de 1 us."
        );
    } else {
        hal_telemetry_write(
            "AVISO: revisar la resolucion observada."
        );
    }
}

/* Comprueba que las funciones antiguas sigan disponibles. */
static void test_legacy_api(void)
{
    char message[120];

    snprintf(
        message,
        sizeof(message),
        "API anterior: millis=%lu | micros=%lu",
        (unsigned long)hal_millis(),
        (unsigned long)hal_micros()
    );

    hal_telemetry_write(message);
}

/* Inicializa el reloj antes de ejecutar las pruebas. */
void test_hal_time_init(void)
{
    clock_ready = hal_time_init();

    if (!clock_ready) {
        hal_telemetry_write(
            "ERROR: fallo hal_time_init()."
        );
        return;
    }

    hal_telemetry_write(
        "HAL TIME inicializada correctamente."
    );
}

/* Ejecuta una serie de pruebas cada dos segundos. */
void test_hal_time_update(void)
{
    if (!clock_ready) {
        return;
    }

    uint32_t now_us = hal_time_us();

    if (!first_run &&
        (uint32_t)(now_us - last_run_us) < TEST_PERIOD_US) {
        return;
    }

    first_run = false;
    last_run_us = now_us;

    hal_telemetry_write("");
    hal_telemetry_write("=== TEST HAL TIME ===");

    test_delay(100U);
    test_delay(10U);
    test_delay(1U);

    test_resolution();
    test_legacy_api();

    hal_telemetry_write("=====================");
}
