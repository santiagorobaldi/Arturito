#include "driver_vl53l0x.h"
#include "hal_telemetry.h"
#include "vl53l0x_api.h"

#include <stdio.h>
#include <string.h>

static VL53L0X_Dev_t sensor;
static bool initialized = false;

static bool check_status(VL53L0X_Error status, const char *step)
{
    if (status == VL53L0X_ERROR_NONE) {
        return true;
    }
    char line[128];
    snprintf(line, sizeof(line), "# VL53L0X: fallo en %s (error %d).",
             step, (int)status);
    hal_telemetry_write(line);
    return false;
}

static vl53l0x_status_t driver_status(VL53L0X_Error status, const char *step)
{
    if (check_status(status, step)) {
        return VL53L0X_OK;
    }

    switch (status) {
    case VL53L0X_ERROR_TIME_OUT:
        return VL53L0X_ERROR_TIMEOUT;
    case VL53L0X_ERROR_RANGE_ERROR:
        return VL53L0X_ERROR_INVALID_MEASUREMENT;
    case VL53L0X_ERROR_CONTROL_INTERFACE:
        return VL53L0X_ERROR_COMMUNICATION;
    default:
        return VL53L0X_ERROR_INTERNAL_API;
    }
}

vl53l0x_status_t vl53l0x_init(void)
{

    uint8_t vhv_settings;
    uint8_t phase_cal;

    VL53L0X_DeviceInfo_t info;

    initialized = false;
    memset(&sensor, 0, sizeof(sensor));
    sensor.I2cDevAddr = 0x52;
    sensor.comms_type = 1;
    sensor.comms_speed_khz = 100;



    if (!check_status(VL53L0X_DataInit(&sensor), "DataInit") ||
        !check_status(VL53L0X_StaticInit(&sensor), "StaticInit") ||
        !check_status(VL53L0X_GetDeviceInfo(&sensor, &info), "GetDeviceInfo") ||
        !check_status(VL53L0X_PerformRefCalibration(&sensor, &vhv_settings, &phase_cal), "RefCalibration") ||
        !check_status(VL53L0X_SetDeviceMode(&sensor,VL53L0X_DEVICEMODE_SINGLE_RANGING), "SetDeviceMode") ||
        !check_status(VL53L0X_SetLimitCheckEnable(&sensor,VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1), "SigmaCheck") ||
        !check_status(VL53L0X_SetLimitCheckEnable(&sensor,VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1), "SignalCheck")) {
        return VL53L0X_ERROR_INIT;
    }


    initialized = true;
    if (vl53l0x_set_profile(VL53L0X_PROFILE_DEFAULT) != VL53L0X_OK) {
        initialized = false;
        return VL53L0X_ERROR_INIT;
    }
    return VL53L0X_OK;
}

vl53l0x_status_t vl53l0x_set_profile(vl53l0x_profile_t profile)
{
    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }
    if ((unsigned int)profile > (unsigned int)VL53L0X_PROFILE_HIGH_ACCURACY) {
        return VL53L0X_ERROR_INIT;
    }

    VL53L0X_Error status = VL53L0X_ERROR_NONE;

    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }

    switch (profile) {
    case VL53L0X_PROFILE_DEFAULT:
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(18 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 14);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 10);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&sensor, 33000);
        }
        break;

    case VL53L0X_PROFILE_LONG_RANGE:
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(60 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.10 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            /* Desactivar RIT: no se aplico calibracion de crosstalk. */
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&sensor, 33000);
        }
        break;

    case VL53L0X_PROFILE_HIGH_SPEED:
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(32 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 14);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 10);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&sensor, 30000);
        }
        break;

    case VL53L0X_PROFILE_HIGH_ACCURACY:
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(18 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 14);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 10);
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&sensor, 200000);
        }
        break;

    default:
        return VL53L0X_ERROR_INIT;
    }

    return driver_status(status, "SetProfile");
}

vl53l0x_status_t vl53l0x_start(void)
{
    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }

    return driver_status(VL53L0X_StartMeasurement(&sensor),
                              "StartMeasurement");
}

vl53l0x_status_t vl53l0x_distance_ready(bool *ready)
{
    if (ready == NULL) {
        return VL53L0X_ERROR_INTERNAL_API;
    }

    *ready = false;
    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }

    uint8_t data_ready = 0;
    vl53l0x_status_t status = driver_status(
        VL53L0X_GetMeasurementDataReady(&sensor, &data_ready), "DataReady");
    if (status == VL53L0X_OK) {
        *ready = data_ready != 0;
    }
    return status;
}

vl53l0x_status_t vl53l0x_get_distance(float *distance_cm)
{
    if (distance_cm == NULL) {
        return VL53L0X_ERROR_INTERNAL_API;
    }

    if (!initialized) {
        return VL53L0X_ERROR_INIT;
    }

    VL53L0X_RangingMeasurementData_t measurement = {0};
    vl53l0x_status_t status = driver_status(
        VL53L0X_GetRangingMeasurementData(&sensor, &measurement), "GetDistance");
    if (status != VL53L0X_OK) {
        return status;
    }

    /* Reconocer el aviso tambien usando polling, aun con muestra invalida. */
    status = driver_status(VL53L0X_ClearInterruptMask(&sensor, 0),"ClearInterrupt");
    if (status != VL53L0X_OK) {
        return status;
    }
    if (measurement.RangeStatus != 0) {
        return VL53L0X_ERROR_INVALID_MEASUREMENT;
    }

    *distance_cm = (float)measurement.RangeMilliMeter / 10.0f;
    return VL53L0X_OK;
}
