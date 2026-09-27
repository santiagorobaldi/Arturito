#include "../config.h"

#ifdef BOARD_ESP32

#include "hal_sensors.h"
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <math.h>

namespace {
  Adafruit_VL53L0X lox = Adafruit_VL53L0X();

  int16_t leer_gz_raw() {
    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x47); // registro GYRO_ZOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom(IMU_I2C_ADDR, 2, true);
    int16_t hi = Wire.read();
    int16_t lo = Wire.read();
    return (int16_t)((hi << 8) | lo);
  }
}

void hal_sensors_init() {
  Wire.begin();

  // Saca al MPU6050 del sleep mode
  Wire.beginTransmission(IMU_I2C_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  if (!lox.begin()) {
    Serial.println("Error: No se encuentra el sensor ToF VL53L0X!");
    while (1);
  }
}

float hal_get_distance_cm() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  if (measure.RangeStatus == 4) return -1;
  return measure.RangeMilliMeter / 10.0f;
}

float hal_get_gyro_z_rads() {
  int16_t gz_raw = leer_gz_raw();
  float gz_dps = gz_raw / 131.0f; // sensibilidad del MPU6050 en rango default
  return gz_dps * (PI / 180.0f);
}

#endif // BOARD_ESP32
