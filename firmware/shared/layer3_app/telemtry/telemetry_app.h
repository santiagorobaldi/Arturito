#ifndef TELEMETRY_APP_H
#define TELEMETRY_APP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void telemetry_app_init(void);
void telemetry_app_update(void);

/* Funciones de alto nivel utilizadas por otros módulos del sistema */
void telemetry_log_info(const char *msg);
void telemetry_plot_value(const char *name, float val);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_APP_H */