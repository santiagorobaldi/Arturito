#include "hal_mpu6050.h"

#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include <stddef.h>

namespace {
Adafruit_MPU6050 sensor;
bool initialized = false;
}


// Adafruit entrega unidades SI sin calibracion automatica de offsets.
bool hal_mpu6050_init(uint8_t address) {
    initialized = false;
    if (address != 0x68U && address != 0x69U) {
        return false;
    }
    Wire.begin();
    Wire.setTimeOut(50);
    if (!sensor.begin(address, &Wire)) {
        return false;
    }
    sensor.setAccelerometerRange(MPU6050_RANGE_16_G);
    sensor.setGyroRange(MPU6050_RANGE_2000_DEG);
    sensor.setFilterBandwidth(MPU6050_BAND_260_HZ);
    initialized = true;
    return true;
}

bool hal_mpu6050_read(mpu6050_sample_t *sample) {
    if (!initialized || sample == NULL) {
        return false;
    }
    sensors_event_t accel, gyro, temperature;
    
    if (!sensor.getEvent(&accel, &gyro, &temperature)) {
        return false;
    }
    mpu6050_sample_t next;
    next.accel_x_mss = accel.acceleration.x;
    next.accel_y_mss = accel.acceleration.y;
    next.accel_z_mss = accel.acceleration.z;
    next.gyro_x_rads = gyro.gyro.x;
    next.gyro_y_rads = gyro.gyro.y;
    next.gyro_z_rads = gyro.gyro.z;
    *sample = next;
    return true;
}
