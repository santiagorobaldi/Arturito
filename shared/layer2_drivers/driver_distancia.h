#ifndef DRIVER_DISTANCIA_H
#define DRIVER_DISTANCIA_H

#ifdef __cplusplus
extern "C" {
#endif

void distancia_init(void);
float distancia_validate_m(float distancia_m);
float distancia_get_m(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_DISTANCIA_H */
