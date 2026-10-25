#ifndef TELEMETRY_DRIVER_H
#define TELEMETRY_DRIVER_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TELEMETRY_LOG_INFO,
    TELEMETRY_LOG_WARN,
    TELEMETRY_LOG_ERROR
} telemetry_log_level_t;

/* Inicializa el driver de telemetría y el HAL subyacente */
bool telemetry_driver_init(void);

/* Procesa eventos de red (debe llamarse en el loop principal) */
void telemetry_driver_poll(void);

/* Envía un mensaje de texto simple con nivel de severidad */
bool telemetry_send_text(telemetry_log_level_t level, const char *message);

/* Envía un dato numérico con clave y valor para graficar */
bool telemetry_send_variable_float(const char *key, float value);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_DRIVER_H */