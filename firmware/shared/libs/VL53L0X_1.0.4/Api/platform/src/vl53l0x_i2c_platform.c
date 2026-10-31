/*
 * COPYRIGHT (C) STMicroelectronics 2015. All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * STMicroelectronics ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with STMicroelectronics
 *
 * Programming Golden Rule: Keep it Simple!
 *
 */

/*!
 * \file   VL53L0X_platform.c
 * \brief  Code function defintions for Ewok Platform Layer
 *
 */



#include <stdio.h>    // sprintf(), vsnprintf(), printf()

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include "vl53l0x_i2c_platform.h"
#include "hal_i2c.h"
#include <string.h>
#include "vl53l0x_def.h"

#include "vl53l0x_platform_log.h"

#ifdef VL53L0X_LOG_ENABLE
#define trace_print(level, ...) trace_print_module_function(TRACE_MODULE_PLATFORM, level, TRACE_FUNCTION_NONE, ##__VA_ARGS__)
#define trace_i2c(...) trace_print_module_function(TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_I2C, ##__VA_ARGS__)
#endif



char  debug_string[VL53L0X_MAX_STRING_LENGTH_PLT];


#define MIN_COMMS_VERSION_MAJOR     1
#define MIN_COMMS_VERSION_MINOR     8
#define MIN_COMMS_VERSION_BUILD     1
#define MIN_COMMS_VERSION_REVISION  0

#define STATUS_OK              0x00
#define STATUS_FAIL            0x01

// PC communications version check is not used by this port.
#if 0
bool_t _check_min_version(void)
{
    /* bool_t min_version_comms_dll = FALSE;
    int32_t status   = STATUS_OK;
    COMMS_VERSION_INFO comms_version_info;


    status = RANGING_SENSOR_COMMS_Get_Version( &comms_version_info );
    if(status != STATUS_OK)
    {
        RANGING_SENSOR_COMMS_Get_Error_Text(debug_string);
    }
    else
    {
        // combined check of major, minor and build
        if (
            (comms_version_info.major > MIN_COMMS_VERSION_MAJOR) ||
            ((comms_version_info.major == MIN_COMMS_VERSION_MAJOR) && (comms_version_info.minor > MIN_COMMS_VERSION_MINOR)) ||
            ((comms_version_info.major == MIN_COMMS_VERSION_MAJOR) && (comms_version_info.minor == MIN_COMMS_VERSION_MINOR) && (comms_version_info.build >= MIN_COMMS_VERSION_BUILD))
            )
        {
            min_version_comms_dll = TRUE;
        }

        // subsequent check of svn revision
        if ( comms_version_info.revision < MIN_COMMS_VERSION_REVISION )
        {
            min_version_comms_dll = FALSE;
        }
    }

    return min_version_comms_dll; */
    return TRUE;
}

#endif

int32_t VL53L0X_comms_initialise(uint8_t comms_type, uint16_t comms_speed_khz)
{
    if (comms_type != I2C || comms_speed_khz == 0U ||
        comms_speed_khz > 400U) {
        return STATUS_FAIL;
    }

    /* The board HAL owns bus speed and pins; reuse an active shared bus. */
    if (hal_i2c_is_initialized(HAL_I2C_BUS_A)) {
        return STATUS_OK;
    }

    return hal_i2c_init(HAL_I2C_BUS_A) ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_comms_close(void)
{
    /*int32_t status = STATUS_OK;

    RANGING_SENSOR_COMMS_Fini_V2W8();

    if(status != STATUS_OK)
    {
        RANGING_SENSOR_COMMS_Get_Error_Text(debug_string);
    }

    return status; */
    return STATUS_OK;
}

int32_t VL53L0X_write_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    uint8_t buffer[VL53L0X_MAX_I2C_XFER_SIZE];
    if (pdata == NULL || count <= 0 || count > VL53L0X_MAX_I2C_DATA_SIZE || address == 0 || (address & 1U) != 0)
        return STATUS_FAIL;

    buffer[0] = index;
    memcpy(&buffer[1], pdata, (size_t)count);

    return hal_i2c_write(HAL_I2C_BUS_A, address >> 1, buffer, (int)(count + 1)) ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata,int32_t count)
{
    if (pdata == NULL || count <= 0 || count > VL53L0X_MAX_I2C_DATA_SIZE ||
        address == 0 || (address & 1U) != 0)
        return STATUS_FAIL;

    
    return hal_i2c_write_read(HAL_I2C_BUS_A, address >> 1, &index, 1, pdata, (int)count) ? STATUS_OK : STATUS_FAIL;
}


int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{

    return VL53L0X_write_multi(address, index, &data, 1);

}


int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    
    uint8_t  buffer[BYTES_PER_WORD];

    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data &  0x00FF);

    return VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);

  

}


int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    
    uint8_t  buffer[BYTES_PER_DWORD];

    
    buffer[0] = (uint8_t) (data >> 24);
    buffer[1] = (uint8_t)((data &  0x00FF0000) >> 16);
    buffer[2] = (uint8_t)((data &  0x0000FF00) >> 8);
    buffer[3] = (uint8_t) (data &  0x000000FF);

    return VL53L0X_write_multi(address, index, buffer, BYTES_PER_DWORD);
    
}


int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
   

    return VL53L0X_read_multi(address, index, pdata, 1);


}


int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    int32_t  status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_WORD];

    if (pdata == NULL) return STATUS_FAIL;
    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_WORD);
    if (status != STATUS_OK) return status;
	*pdata = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1];

    return status;

}

int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    int32_t status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_DWORD];

    if (pdata == NULL) return STATUS_FAIL;
    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_DWORD);
    if (status != STATUS_OK) return status;
    *pdata = ((uint32_t)buffer[0]<<24) + ((uint32_t)buffer[1]<<16) + ((uint32_t)buffer[2]<<8) + (uint32_t)buffer[3];

    return status;

}

int32_t VL53L0X_platform_wait_us(int32_t wait_us)
{
    /* int32_t status = STATUS_OK;
    float wait_ms = (float)wait_us/1000.0f;

    *
     * Use windows event handling to perform non-blocking wait.
     *
    HANDLE hEvent = CreateEvent(0, TRUE, FALSE, 0);
    WaitForSingleObject(hEvent, (int)(wait_ms + 0.5f));

#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("Wait us : %6d\n", wait_us);
#endif

    return status; */
    (void)wait_us;
    return STATUS_FAIL;

}


int32_t VL53L0X_wait_ms(int32_t wait_ms)
{
    /* int32_t status = STATUS_OK;

    *
     * Use windows event handling to perform non-blocking wait.
     *
    HANDLE hEvent = CreateEvent(0, TRUE, FALSE, 0);
    WaitForSingleObject(hEvent, wait_ms);

#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("Wait ms : %6d\n", wait_ms);
#endif

    return status;
    */
    (void)wait_ms;
    return STATUS_FAIL;

}


int32_t VL53L0X_set_gpio(uint8_t level)
{
    /* int32_t status = STATUS_OK;
    //status = VL53L0X_set_gpio_sv(level);
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// Set GPIO = %d;\n", level);
#endif

    return status;
    */
    (void)level;
    return STATUS_FAIL;

}


int32_t VL53L0X_get_gpio(uint8_t *plevel)
{
     /*
    int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// Get GPIO = %d;\n", *plevel);
#endif
    return status;
    */
    (void)plevel;
    return STATUS_FAIL;
}


int32_t VL53L0X_release_gpio(void)
{
    /*
    int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// Releasing force on GPIO\n");
#endif
    return status;
    */
    return STATUS_FAIL;

}

int32_t VL53L0X_cycle_power(void)
{
   /* int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// cycle sensor power\n");
#endif
    status = RANGING_SENSOR_COMMS_Cycle_Sensor_Power();

	if(status != STATUS_OK)
    {
        RANGING_SENSOR_COMMS_Get_Error_Text(debug_string);
    }

	return status;
    */
    return STATUS_FAIL;
}


int32_t VL53L0X_get_timer_frequency(int32_t *ptimer_freq_hz)
{
       if (ptimer_freq_hz != NULL) *ptimer_freq_hz = 0;
       return STATUS_FAIL;
}


int32_t VL53L0X_get_timer_value(int32_t *ptimer_count)
{
       if (ptimer_count != NULL) *ptimer_count = 0;
       return STATUS_FAIL;
}
