#ifndef HAL_VL53L0X_H
#define HAL_VL53L0X_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/** Inicializa y calibra el sensor. Puede bloquear durante el arranque.
 * En EDU-CIAA requiere I2C0 ya preparado por la aplicacion.
 */
bool hal_vl53l0x_init(void);

/** Obtiene una distancia en metros; devuelve -1 si no hay una lectura valida.
 * Es una operacion bloqueante. Serializar el acceso al dispositivo y al bus.
 * No entrega una marca temporal sincronizada con la IMU.
 */
float hal_vl53l0x_get_distance_cm(void);

#if defined(BOARD_ESP32)
/** Contadores de diagnóstico de las lecturas ToF desde el último reset. */
typedef struct {
    uint16_t api_errors;
    uint16_t range_status_counts[6]; /* Estados 0 a 5. */
    uint16_t other_status_count;
    uint16_t sentinel_count;
} hal_vl53l0x_diagnostics_t;

void hal_vl53l0x_reset_diagnostics(void);
void hal_vl53l0x_get_diagnostics(hal_vl53l0x_diagnostics_t *diagnostics);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HAL_VL53L0X_H */
