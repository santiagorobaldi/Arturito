#include "driver_vl53l0x.h"
#include "config.h"
#include "hal_vl53l0x.h"

bool vl53l0x_init(void) {
    return hal_vl53l0x_init();
}

float vl53l0x_validate_m(float distancia_m) {
    if (distancia_m < DIST_MIN_M || distancia_m > DIST_MAX_M) {
        return -1.0f;
    }
    return distancia_m;
}

float vl53l0x_get_distance_m(void) {
    return vl53l0x_validate_m(hal_vl53l0x_get_distance_m());
}
