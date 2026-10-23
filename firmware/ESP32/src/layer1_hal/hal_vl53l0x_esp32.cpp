#include "hal_vl53l0x.h"

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

namespace {
    Adafruit_VL53L0X sensor;
    bool vl53l0x_ok = false;
}

/* La aplicacion recibe nuestros estados; el codigo ST queda en el log. */
static hal_vl53l0x_status_t measurement_status(VL53L0X_Error status,
                                             const char *step)
{
    if (status == VL53L0X_ERROR_NONE) {
        return HAL_VL53L0X_OK;
    }
    Serial.printf("# VL53L0X: fallo en %s (error %d).\r\n", step, (int)status);
    if (status == VL53L0X_ERROR_TIME_OUT) {
        return HAL_VL53L0X_ERROR_TIMEOUT;
    }
    if (status == VL53L0X_ERROR_RANGE_ERROR) {
        return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    }
    return HAL_VL53L0X_ERROR_COMMUNICATION;
}

hal_vl53l0x_status_t hal_vl53l0x_init(void)
{
    vl53l0x_ok = false;
    Wire.begin();
    if (!sensor.begin()) {
        Serial.printf("# VL53L0X: fallo en begin (error %d).\r\n",
                      (int)sensor.Status);
        return HAL_VL53L0X_ERROR_INIT;
    }
    if (measurement_status(
            sensor.setDeviceMode(VL53L0X_DEVICEMODE_SINGLE_RANGING, false),
            "SetDeviceMode") != HAL_VL53L0X_OK) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    vl53l0x_ok = true;
    if (hal_vl53l0x_set_profile(HAL_VL53L0X_PROFILE_DEFAULT) != HAL_VL53L0X_OK) {
        vl53l0x_ok = false;
        return HAL_VL53L0X_ERROR_INIT;
    }
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_start(void)
{
    if (!vl53l0x_ok) {
        return HAL_VL53L0X_ERROR_INIT;
    }
    return measurement_status(sensor.startMeasurement(false), "StartMeasurement");
}

hal_vl53l0x_status_t hal_vl53l0x_distance_ready(bool *ready)
{
    *ready = false;
    if (!vl53l0x_ok) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    /* Adafruit devuelve true tanto por dato listo como por error. */
    bool complete = sensor.isRangeComplete();
    hal_vl53l0x_status_t status = measurement_status(sensor.Status, "DataReady");
    if (status == HAL_VL53L0X_OK) {
        *ready = complete;
    }
    return status;
}

hal_vl53l0x_status_t hal_vl53l0x_get_distance(float *distance_m)
{
    if (!vl53l0x_ok) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    VL53L0X_RangingMeasurementData_t measurement = {};
    hal_vl53l0x_status_t status = measurement_status(
        sensor.getRangingMeasurement(&measurement, false), "GetDistance");
    if (status != HAL_VL53L0X_OK) {
        return status;
    }

    /* Consumir el aviso incluso cuando la calidad de la muestra es invalida. */
    status = measurement_status(sensor.clearInterruptMask(false), "ClearInterrupt");
    if (status != HAL_VL53L0X_OK) {
        return status;
    }
    if (measurement.RangeStatus != 0) {
        return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    }

    *distance_m = measurement.RangeMilliMeter / 1000.0f;
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_set_profile(hal_vl53l0x_profile_t profile)
{
    if (!vl53l0x_ok) {
        return HAL_VL53L0X_ERROR_INIT;
    }
    if (profile < HAL_VL53L0X_PROFILE_DEFAULT ||
        profile > HAL_VL53L0X_PROFILE_HIGH_ACCURACY) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    /* Cada perfil restaura todos los ajustes para no heredar el anterior.
     * Valores basados en Adafruit 1.2.5; presupuesto aplicado al final.
     */
    if (status == VL53L0X_ERROR_NONE) {
        sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
        status = sensor.Status;
    }
    if (status == VL53L0X_ERROR_NONE) {
        sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
        status = sensor.Status;
    }

    switch (profile) {
    case HAL_VL53L0X_PROFILE_DEFAULT:
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(18 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, 14);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 10);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setMeasurementTimingBudgetMicroSeconds(33000);
            status = sensor.Status;
        }
        break;

    case HAL_VL53L0X_PROFILE_LONG_RANGE:
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(60 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.10 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setMeasurementTimingBudgetMicroSeconds(33000);
            status = sensor.Status;
        }
        break;

    case HAL_VL53L0X_PROFILE_HIGH_SPEED:
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(32 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, 14);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 10);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setMeasurementTimingBudgetMicroSeconds(30000);
            status = sensor.Status;
        }
        break;

    case HAL_VL53L0X_PROFILE_HIGH_ACCURACY:
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(18 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckValue(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setLimitCheckEnable(VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_PRE_RANGE, 14);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setVcselPulsePeriod(VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 10);
            status = sensor.Status;
        }
        if (status == VL53L0X_ERROR_NONE) {
            sensor.setMeasurementTimingBudgetMicroSeconds(200000);
            status = sensor.Status;
        }
        break;

    default:
        return HAL_VL53L0X_ERROR_INIT;
    }

    return measurement_status(status, "SetProfile");
}
