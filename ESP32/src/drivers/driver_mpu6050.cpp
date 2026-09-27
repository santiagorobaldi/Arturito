#include "driver_mpu6050.h"
#include "../hal/hal_sensors.h"
#include "../config.h"
#include <math.h>

// Nota: el timing (millis) sigue siendo de Arduino porque no forma parte
// del contrato de capa 1 (es un detalle menor, pero si portás a EDU-CIAA
// vas a necesitar reemplazarlo por el tick de sAPI).
#if defined(BOARD_ESP32)
  #include <Arduino.h>
#endif

namespace {
  float gyroZ_offset_rads = 0;
  float theta_rad = 0;
  unsigned long tiempo_previo = 0;
  float gz_rads_previo = 0.0f;
}

void mpu6050_init() {
  // La inicialización real del sensor la hace hal_sensors_init() en main().
}

void mpu6050_calibrate() {
  delay(2000);
  float suma = 0;
  for (int i = 0; i < 1000; i++) {
    suma += hal_get_gyro_z_rads();
    delay(2);
  }
  gyroZ_offset_rads = suma / 1000.0f;
  tiempo_previo = millis();
}

void mpu6050_update() {
  float gz_rads = hal_get_gyro_z_rads() - gyroZ_offset_rads;

  unsigned long tiempo_actual = millis();
  float dt = (tiempo_actual - tiempo_previo) / 1000.0f;
  tiempo_previo = tiempo_actual;

  float deadband_rads = 1.0f * (PI / 180.0f); // mismo umbral que antes, en rad/s
  if (fabs(gz_rads) <= deadband_rads) gz_rads = 0.0f;

  // Integración trapezoidal, igual que antes
  float velocidad_promedio = (gz_rads + gz_rads_previo) / 2.0f;
  theta_rad += velocidad_promedio * dt;
  gz_rads_previo = gz_rads;
}

float mpu6050_get_theta_rad() {
  return theta_rad;
}
