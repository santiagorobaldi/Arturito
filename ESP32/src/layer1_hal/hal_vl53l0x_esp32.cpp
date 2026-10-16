#include "hal_vl53l0x.h"

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

namespace {
    Adafruit_VL53L0X sensor = Adafruit_VL53L0X();
    bool vl53l0x_ok = false;
}

bool hal_vl53l0x_init(void) {
    Wire.begin();
    vl53l0x_ok = sensor.begin();
    return vl53l0x_ok;
}

float hal_vl53l0x_get_distance_m(void) {
    if (!vl53l0x_ok) {
        return -1.0f;
    }
    VL53L0X_RangingMeasurementData_t measure;
    sensor.rangingTest(&measure, false);
    if (measure.RangeStatus == 4) {
        return -1.0f;
    }
    return measure.RangeMilliMeter / 1000.0f;
}
