#ifndef ANGLE_H
#define ANGLE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializa gyro Z y calcula su bias con el sensor quieto. */
bool angle_init(uint8_t imu_address);

/* Actualiza y devuelve el angulo integrado en grados. */
bool angle_read_deg(double *angle_deg);

#ifdef __cplusplus
}
#endif

#endif /* ANGLE_H */
