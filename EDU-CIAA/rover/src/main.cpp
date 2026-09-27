#include "sapi.h"
#include "hal_sensors.h"
#include "driver_mpu6050.h"
#include "driver_distancia.h"
#include "config.h"

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void) {
  boardConfig();

  hal_sensors_init();
  mpu6050_calibrate();
  distancia_init();

  while (TRUE) {
    mpu6050_update();

    float theta_rad = mpu6050_get_theta_rad();
    float distancia_cm = distancia_get_cm();

    // Mismo formato "theta,distancia" que usa el visualizador de Processing.
    printf("%f,%f\r\n", theta_rad, distancia_cm);

    delay(SAMPLE_PERIOD_MS);
  }

  return 0;
}
