#include "config.h"
#include "hal_imu.h"

#include <Wire.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif

namespace {
    bool imu_ok = false;

    void i2c_ensure(void) {
        Wire.begin();
    }

    int16_t leer_gz_raw(void) {
        Wire.beginTransmission(IMU_I2C_ADDR);
        Wire.write(0x47);
        Wire.endTransmission(false);
        Wire.requestFrom((uint16_t)IMU_I2C_ADDR, (size_t)2, true);
        if (Wire.available() < 2) {
            return 0;
        }
        int16_t hi = Wire.read();
        int16_t lo = Wire.read();
        return (int16_t)((hi << 8) | lo);
    }
}

bool hal_imu_init(void) {
    i2c_ensure();

    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x6B);
    Wire.write(0x00);
    if (Wire.endTransmission(true) != 0) {
        imu_ok = false;
        return false;
    }

    Wire.beginTransmission(IMU_I2C_ADDR);
    Wire.write(0x75);
    Wire.endTransmission(false);
    Wire.requestFrom((uint16_t)IMU_I2C_ADDR, (size_t)1, true);
    if (Wire.available() < 1) {
        imu_ok = false;
        return false;
    }
    uint8_t who = (uint8_t)Wire.read();
    if (who == 0x00 || who == 0xFF) {
        imu_ok = false;
        return false;
    }

    imu_ok = true;
    return true;
}

float hal_imu_get_gyro_z_rads(void) {
    if (!imu_ok) {
        return 0.0f;
    }
    int16_t gz_raw = leer_gz_raw();
    float gz_dps = gz_raw / 131.0f;
    return gz_dps * ((float)PI / 180.0f);
}
