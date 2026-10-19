#ifndef HAL_TELEMETRY_NETWORK_H
#define HAL_TELEMETRY_NETWORK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Arranca el intento de conexión; no espera a que el router responda. */
bool hal_telemetry_network_start(void);

/* Llamar seguido desde loop() para atender la red y aceptar clientes. */
void hal_telemetry_network_poll(void);

/*
 * Envía una línea ASCII terminada en '\n' (hasta 127 bytes).
 * Devuelve false si no hay cliente o el envío no se completa.
 */
bool hal_telemetry_network_send_line(const char *line);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TELEMETRY_NETWORK_H */
