#ifndef PERCEPTION_V0_H
#define PERCEPTION_V0_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Inicia el ensayo de percepcion: orientacion y distancia por serie.
 * Crea una tarea que inicializa sensores, calibra giro y prepara adquisicion,
 * procesamiento y publicacion. El formato es theta_rad,distancia_m.
 * Devuelve true si pudo crear la tarea de arranque.
 * Los resultados posteriores de inicializacion se informan por telemetria.
 * Invocar una sola vez durante el arranque del programa. */
bool perception_v0_start(void);

#ifdef __cplusplus
}
#endif

#endif /* PERCEPTION_V0_H */
