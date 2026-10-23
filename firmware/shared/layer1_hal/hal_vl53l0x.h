#ifndef HAL_VL53L0X_H
#define HAL_VL53L0X_H

#include <stdbool.h>

typedef enum {
    HAL_VL53L0X_OK = 0,
    HAL_VL53L0X_ERROR_INIT,
    HAL_VL53L0X_ERROR_TIMEOUT,
    HAL_VL53L0X_ERROR_INVALID_MEASUREMENT,
    HAL_VL53L0X_ERROR_COMMUNICATION
} hal_vl53l0x_status_t;

typedef enum {
    HAL_VL53L0X_PROFILE_DEFAULT = 0,
    HAL_VL53L0X_PROFILE_LONG_RANGE,
    HAL_VL53L0X_PROFILE_HIGH_SPEED,
    HAL_VL53L0X_PROFILE_HIGH_ACCURACY
} hal_vl53l0x_profile_t;

#ifdef __cplusplus
extern "C" {
#endif


/** Inicializa tras reset del sensor, calibra referencia y aplica DEFAULT.
 * Puede bloquear durante el arranque. El bus debe estar preparado.
 */
hal_vl53l0x_status_t hal_vl53l0x_init(void);

/** Cambia el perfil entre mediciones, antes del siguiente start.
 * Requiere init exitoso y uno de los perfiles del enum.
 */
hal_vl53l0x_status_t hal_vl53l0x_set_profile(hal_vl53l0x_profile_t profile);

/** Inicia una medicion single sin esperar su resultado completo.
 * Requiere inicializacion exitosa y haber recogido la medicion anterior.
 */
hal_vl53l0x_status_t hal_vl53l0x_start(void);

/** Consulta tras start: Estado OK con *ready=false significa pendiente, no error.
 * ready debe ser distinto de NULL. Una consulta no espera toda la conversion.
 * La espera entre consultas corresponde al llamador; no hay plazo acumulado.
 */
hal_vl53l0x_status_t hal_vl53l0x_distance_ready(bool *ready);

/** Recoge y reconoce la muestra tras distance_ready con ready=true.
 * distance_m debe ser distinto de NULL; se escribe solo si devuelve OK.
 * Rechaza muestras que el sensor considera invalidas. No espera conversion.
 */
hal_vl53l0x_status_t hal_vl53l0x_get_distance(float *distance_m);

#ifdef __cplusplus
}
#endif

#endif /* HAL_VL53L0X_H */
