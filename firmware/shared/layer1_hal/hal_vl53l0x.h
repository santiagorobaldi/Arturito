#ifndef HAL_VL53L0X_H
#define HAL_VL53L0X_H

#include <stdbool.h>

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
float hal_vl53l0x_get_distance_m(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_VL53L0X_H */
