#include "driver_distancia.h"
#include "hal_sensors.h"

static const float DIST_MIN_CM = 5.0f;
static const float DIST_MAX_CM = 150.0f;

void distancia_init() {
    // Inicialización delegada a hal_sensors_init()
}

float distancia_get_cm() {
    float cm = hal_get_distance_cm();
    if (cm < DIST_MIN_CM || cm > DIST_MAX_CM) return -1.0f;
    return cm;
}