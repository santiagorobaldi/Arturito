#ifndef HAL_TELEMETRY_NETWORK_H
#define HAL_TELEMETRY_NETWORK_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inicia el AP de Arturito y el servidor TCP. */
bool hal_telemetry_network_start(void);

/* Llamar desde loop() para aceptar clientes. */
void hal_telemetry_network_poll(void);

/* Envia una linea ASCII terminada en '\n' (maximo 127 bytes). */
bool hal_telemetry_network_send_line(const char *line);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TELEMETRY_NETWORK_H */
