#include "hal_vl53l0x.h"
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
    printf("# VL53L0X: fallo en %s (error %d).\r\n", step, (int)status);
    return false;
}

bool hal_vl53l0x_init(void)
{
    uint32_t spad_count;
    uint8_t aperture_spads;
    uint8_t vhv_settings;
    uint8_t phase_cal;
    VL53L0X_DeviceInfo_t info;

    initialized = false;
    memset(&sensor, 0, sizeof(sensor));
    sensor.I2cDevAddr = 0x52;
    sensor.comms_type = 1;
    sensor.comms_speed_khz = 400;
    sensor.i2cBus = I2C0;

    /* Runtime initializes the IMU first; its sAPI backend prepares I2C0.
     * Do not reset that shared bus here. Standalone users must prepare it.
     */
    if (!check_status(VL53L0X_DataInit(&sensor), "DataInit") ||
        !check_status(VL53L0X_GetDeviceInfo(&sensor, &info), "GetDeviceInfo") ||
        !check_status(VL53L0X_StaticInit(&sensor), "StaticInit") ||
        !check_status(VL53L0X_PerformRefCalibration(&sensor, &vhv_settings,
                                                  &phase_cal), "RefCalibration") ||
        !check_status(VL53L0X_PerformRefSpadManagement(&sensor, &spad_count,
                                                     &aperture_spads), "RefSpadManagement") ||
        !check_status(VL53L0X_SetDeviceMode(&sensor,
                          VL53L0X_DEVICEMODE_SINGLE_RANGING), "SetDeviceMode") ||
        !check_status(VL53L0X_SetLimitCheckEnable(&sensor,
                          VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1), "SigmaCheck") ||
        !check_status(VL53L0X_SetLimitCheckEnable(&sensor,
                          VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1), "SignalCheck")) {
        return false;
    }

    printf("# VL53L0X: API ST inicializada (revision %u.%u).\r\n",
           (unsigned)info.ProductRevisionMajor,
           (unsigned)info.ProductRevisionMinor);
    initialized = true;
    return true;
}

float hal_vl53l0x_get_distance_m(void)
{
    VL53L0X_RangingMeasurementData_t measurement = {0};
    if (!initialized) {
        return -1.0f;
    }

    /* Blocking single-shot, including ST's measurement-ready polling.
     * Timing and association with IMU orientation await hardware validation.
     */
    VL53L0X_Error status = VL53L0X_PerformSingleRangingMeasurement(
        &sensor, &measurement);
    if (status != VL53L0X_ERROR_NONE || measurement.RangeStatus != 0) {
        return -1.0f;
    }
    return (float)measurement.RangeMilliMeter / 1000.0f;
}
