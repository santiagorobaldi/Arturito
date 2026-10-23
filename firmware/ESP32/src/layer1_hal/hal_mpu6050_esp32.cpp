#include "hal_mpu6050.h"

#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include <stddef.h>

namespace {
Adafruit_MPU6050 sensor;
bool initialized = false;
}

bool hal_mpu6050_init(uint8_t address) {
    initialized = false;
    if (address != 0x68U && address != 0x69U) {
        return false;
    }

    Wire.begin();
    Wire.setClock(400000U);
    Wire.setTimeOut(10U);

    if (!sensor.begin(address, &Wire)) {
        return false;
    }

    sensor.setGyroRange(MPU6050_RANGE_250_DEG);
    sensor.setFilterBandwidth(MPU6050_BAND_44_HZ);
    /* DLPF activo: gyro ODR = 1 kHz / (1 + divisor) = 100 Hz. */
    sensor.setSampleRateDivisor(9U);
    initialized = true;
    return true;
}

bool hal_mpu6050_read_gyro_z(float *gyro_z_rads) {
    if (!initialized || gyro_z_rads == NULL) {
        return false;
    }

    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temperature;
    if (!sensor.getEvent(&accel, &gyro, &temperature)) {
        return false;
    }

    *gyro_z_rads = gyro.gyro.z;
    return true;
}
