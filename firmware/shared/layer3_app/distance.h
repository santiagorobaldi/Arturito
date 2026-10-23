#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inicia el ToF en el perfil usado por el ensayo. */
bool distance_init(void);

/* Valida una lectura cruda, devuelve cm o -1. */
float distance_validate_raw(float raw_cm);

/* Adquiere una lectura sin aplicar la tabla de calibracion. */
float distance_get_raw(void);

/* Interpola la lectura cruda dentro del rango cubierto por la tabla. */
float distance_calibrate(float raw_cm);

/* Devuelve la distancia validada y calibrada en cm, o -1 ante error. */
float distance_get(void);

#ifdef __cplusplus
}
#endif

#endif /* DISTANCE_H */
