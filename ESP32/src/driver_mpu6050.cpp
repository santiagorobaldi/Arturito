#include "driver_mpu6050.h"
#include "hal_sensors.h"
#include "config.h"
//#include "sapi.h"
#include <math.h>

namespace {
  const float PI_LOCAL = 3.14159265358979323846f;
  float gyroZ_offset_rads = 0;
  float theta_rad = 0;
  float gz_rads_previo = 0.0f;

  // Antes usábamos millis() para medir el dt real entre lecturas. Como
  // ninguna de las dos placas tiene un timestamp común confirmado (Arduino
  // vs sAPI), y el loop que llama a esto ya duerme SAMPLE_PERIOD_MS entre
  // vuelta y vuelta, asumimos ese período como dt fijo. Es menos preciso
  // que medir el tiempo real (se desincroniza si alguna lectura tarda de
  // más), pero deja este driver sin ninguna dependencia de plataforma.
  const float DT_S = SAMPLE_PERIOD_MS / 1000.0f;
}

void mpu6050_calibrate() {
  float suma = 0;
  const int N = 200; // menos muestras que antes para no demorar tanto sin medir tiempo real
  for (int i = 0; i < N; i++) {
    suma += hal_get_gyro_z_rads();
    //delay(2);
  }
  gyroZ_offset_rads = suma / N;
}

void mpu6050_update() {
  float gz_rads = hal_get_gyro_z_rads() - gyroZ_offset_rads;

  float deadband_rads = 1.0f * (PI_LOCAL / 180.0f);
  if (fabs(gz_rads) <= deadband_rads) gz_rads = 0.0f;

  float velocidad_promedio = (gz_rads + gz_rads_previo) / 2.0f;
  theta_rad += velocidad_promedio * DT_S;
  gz_rads_previo = gz_rads;
}

float mpu6050_get_theta_rad() {
  return theta_rad;
}
