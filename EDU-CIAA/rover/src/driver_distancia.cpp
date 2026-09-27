#include "driver_distancia.h"
#include "hal_sensors.h"

namespace {
  const float DIST_MIN_CM = 5.0f;
  const float DIST_MAX_CM = 150.0f;
}

void distancia_init() {}

float distancia_get_cm() {
  float cm = hal_get_distance_cm();
  if (cm < DIST_MIN_CM || cm > DIST_MAX_CM) return -1;
  return cm;
}
