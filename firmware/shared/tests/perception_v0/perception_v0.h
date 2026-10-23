#ifndef PERCEPTION_V0_H
#define PERCEPTION_V0_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Inicializa sensores y calibra gyro Z con el robot quieto.
 * El formato publicado es theta_deg,distancia_cm. Invocar una sola vez. */
bool perception_v0_init(void);

/** Adquiere y publica una lectura. Llamar repetidamente desde el while/loop
 * principal; incluye la pausa configurada entre lecturas. */
void perception_v0_step(void);

#ifdef __cplusplus
}
#endif

#endif /* PERCEPTION_V0_H */
