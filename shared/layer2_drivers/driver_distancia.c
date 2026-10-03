#include "driver_distancia.h"
#include "config.h"
#include "hal_tof.h"

void distancia_init(void) {
}

float distancia_validate_m(float distancia_m) {
    if (distancia_m < DIST_MIN_M || distancia_m > DIST_MAX_M) {
        return -1.0f;
    }
    return distancia_m;
}

float distancia_get_m(void) {
    return distancia_validate_m(hal_tof_get_distance_m());
}
