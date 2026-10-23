#include "hal_vl53l0x.h"
#include "vl53l0x_api.h"

#include <string.h>

static VL53L0X_Dev_t sensor;
static bool initialized;

static bool check_status(VL53L0X_Error status, const char *step)
{
    if (status == VL53L0X_ERROR_NONE) {
        return true;
    }
    printf("# VL53L0X: fallo en %s (error %d).\r\n", step, (int)status);
    return false;
}

/* Errores de adquisicion: conservar codigo ST y operacion en el diagnostico. */
static hal_vl53l0x_status_t measurement_status(VL53L0X_Error status,
                                             const char *step)
{
    if (check_status(status, step)) {
        return HAL_VL53L0X_OK;
    }
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
        return HAL_VL53L0X_ERROR_INIT;
    }


    initialized = true;
    if (hal_vl53l0x_set_profile(HAL_VL53L0X_PROFILE_DEFAULT) != HAL_VL53L0X_OK) {
        initialized = false;
        return HAL_VL53L0X_ERROR_INIT;
    }
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_start(void)
{
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    return measurement_status(VL53L0X_StartMeasurement(&sensor),
                              "StartMeasurement");
}

hal_vl53l0x_status_t hal_vl53l0x_distance_ready(bool *ready)
{
    *ready = false;
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    uint8_t data_ready = 0;
    hal_vl53l0x_status_t status = measurement_status(
        VL53L0X_GetMeasurementDataReady(&sensor, &data_ready), "DataReady");
    if (status == HAL_VL53L0X_OK) {
        *ready = data_ready != 0;
    }
    return status;
}

hal_vl53l0x_status_t hal_vl53l0x_get_distance(float *distance_m)
{
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    VL53L0X_RangingMeasurementData_t measurement = {0};
    hal_vl53l0x_status_t status = measurement_status(
        VL53L0X_GetRangingMeasurementData(&sensor, &measurement), "GetDistance");
    if (status != HAL_VL53L0X_OK) {
        return status;
    }

    /* Reconocer el aviso tambien usando polling, aun con muestra invalida. */
    status = measurement_status(VL53L0X_ClearInterruptMask(&sensor, 0),
                                "ClearInterrupt");
    if (status != HAL_VL53L0X_OK) {
        return status;
    }
    if (measurement.RangeStatus != 0) {
        return HAL_VL53L0X_ERROR_INVALID_MEASUREMENT;
    }

    *distance_m = (float)measurement.RangeMilliMeter / 1000.0f;
    return HAL_VL53L0X_OK;
}

hal_vl53l0x_status_t hal_vl53l0x_set_profile(hal_vl53l0x_profile_t profile)
{
    if (!initialized) {
        return HAL_VL53L0X_ERROR_INIT;
    }
    if (profile < HAL_VL53L0X_PROFILE_DEFAULT ||
        profile > HAL_VL53L0X_PROFILE_HIGH_ACCURACY) {
        return HAL_VL53L0X_ERROR_INIT;
    }

    VL53L0X_Error status = VL53L0X_ERROR_NONE;
    
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }
    if (status == VL53L0X_ERROR_NONE) {
        status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }

    switch (profile) {
    case HAL_VL53L0X_PROFILE_DEFAULT:
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

    case HAL_VL53L0X_PROFILE_LONG_RANGE:
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(60 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.10 * 65536));
        }
        if (status == VL53L0X_ERROR_NONE) {
            status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));
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

    case HAL_VL53L0X_PROFILE_HIGH_SPEED:
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

    case HAL_VL53L0X_PROFILE_HIGH_ACCURACY:
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
        return HAL_VL53L0X_ERROR_INIT;
    }

    return measurement_status(status, "SetProfile");
}
