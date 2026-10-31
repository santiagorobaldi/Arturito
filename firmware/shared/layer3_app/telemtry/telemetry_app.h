#ifndef TELEMETRY_APP_H
#define TELEMETRY_APP_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
bool telemetry_app_init(void);
void telemetry_app_update(void);
bool telemetry_app_send_line(const char *line);
#ifdef __cplusplus
}
#endif
#endif
