#include "main.h"
#include "driver_encoder.h"
#include "hal_i2c.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include "hal_time.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ENCODER_TEST_PERIOD_US 100000U

static bool encoder_ready[2];
static bool clock_ready;
static bool first_sample;
static uint32_t last_sample_us;

static const char *status_name(encoder_status_t status)
{
    switch (status) {
    case ENCODER_OK: return "OK";
    case ENCODER_ERROR_ARGUMENT: return "argumento invalido";
    case ENCODER_ERROR_NOT_INITIALIZED: return "no inicializado";
    case ENCODER_ERROR_COMMUNICATION: return "error de comunicacion";
    case ENCODER_ERROR_INTERNAL_API: return "error interno de API";
    default: return "estado desconocido";
    }
}

static void report_status(const char *name, const char *operation,
                          encoder_status_t status)
{
    char message[120];
    snprintf(message, sizeof(message), "%s %s: %s (%d)",
             name, operation, status_name(status), (int)status);
    telemetry_app_send_line(message);
}

static void init_encoder(encoder_t encoder, hal_i2c_bus_t bus,
                         const char *name)
{
    if (!hal_i2c_init(bus)) {
        char message[80];
        snprintf(message, sizeof(message), "%s: error al inicializar I2C", name);
        telemetry_app_send_line(message);
        return;
    }

    const encoder_status_t status = encoder_init(encoder);
    report_status(name, "inicio del driver (no verifica el sensor)", status);
    if (status != ENCODER_OK) {
        return;
    }

    /* El driver puede inicializarse aunque no haya un sensor conectado. */
    float angle_rad = 0.0f;
    const encoder_status_t read_status = encoder_get_angle_rad(encoder, &angle_rad);
    report_status(name, "primera lectura real", read_status);
    encoder_ready[encoder] = (read_status == ENCODER_OK);
    if (!encoder_ready[encoder]) {
        telemetry_app_send_line("Encoder sin lectura valida; se omite hasta reiniciar.");
    }
}

static void read_encoder(encoder_t encoder, const char *name)
{
    if (!encoder_ready[encoder]) {
        return;
    }

    float angle_rad = 0.0f;
    const encoder_status_t status = encoder_get_angle_rad(encoder, &angle_rad);
    if (status != ENCODER_OK) {
        report_status(name, "lectura", status);
        return;
    }

    /* Formato entero para no depender del soporte de printf para floats. */
    const uint32_t millirad = (uint32_t)(angle_rad * 1000.0f + 0.5f);
    char message[80];
    snprintf(message, sizeof(message), "%s: %lu.%03lu rad",
             name, (unsigned long)(millirad / 1000U),
             (unsigned long)(millirad % 1000U));
    telemetry_app_send_line(message);
}

void app_init(void)
{
    if (!telemetry_app_init()) {
        hal_telemetry_write("ERROR: no se pudo iniciar la telemetria.");
    }

    encoder_ready[ENC_IZQ] = false;
    encoder_ready[ENC_DER] = false;
    first_sample = true;
    last_sample_us = 0U;
    telemetry_app_send_line("=== TEST ENCODERS AS5600 ===");

    clock_ready = hal_time_init();
    if (!clock_ready) {
        telemetry_app_send_line("ERROR: no se pudo inicializar el reloj; test detenido.");
        return;
    }

    init_encoder(ENC_IZQ, HAL_I2C_BUS_A, "IZQ (bus A)");
    init_encoder(ENC_DER, HAL_I2C_BUS_B, "DER (bus B)");
    telemetry_app_send_line("Girar las ruedas a mano: angulo absoluto entre 0 y 2*pi rad.");
    telemetry_app_send_line("Si fallo el inicio de un encoder, reiniciar para reintentar.");
}

void app_update(void)
{
    telemetry_app_update();

    if (!clock_ready || (!encoder_ready[ENC_IZQ] && !encoder_ready[ENC_DER])) {
        return;
    }

    const uint32_t now_us = hal_time_us();
    if (!first_sample &&
        (uint32_t)(now_us - last_sample_us) < ENCODER_TEST_PERIOD_US) {
        return;
    }

    first_sample = false;
    last_sample_us = now_us;
    read_encoder(ENC_IZQ, "IZQ");
    read_encoder(ENC_DER, "DER");
}
