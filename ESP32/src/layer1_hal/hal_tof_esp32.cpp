#include "hal_tof.h"

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

namespace {
    Adafruit_VL53L0X lox = Adafruit_VL53L0X();
    bool tof_ok = false;
}

bool hal_tof_init(void) {
    Wire.begin();
    tof_ok = lox.begin();
    return tof_ok;
}

float hal_tof_get_distance_m(void) {
    if (!tof_ok) {
        return -1.0f;
    }
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    if (measure.RangeStatus == 4) {
        return -1.0f;
    }
    return measure.RangeMilliMeter / 1000.0f;
}
