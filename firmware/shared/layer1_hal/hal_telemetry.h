#ifndef HAL_TELEMETRY_H
#define HAL_TELEMETRY_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool hal_telemetry_init(void);
void hal_telemetry_update(void);
/* Texto sin salto final; el backend agrega LF. true no implica ACK de la PC. */
bool hal_telemetry_send_line(const char *line);
/* Diagnostico por USB. */
void hal_telemetry_write(const char *line);

#ifdef __cplusplus
}
#endif
#endif
