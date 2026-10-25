#ifndef HAL_TELEMETRY_H
#define HAL_TELEMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Envia una linea por el transporte de telemetria activo en la placa. */
void hal_telemetry_write(const char *line);

#ifdef __cplusplus
}
#endif

#endif /* HAL_TELEMETRY_H */


