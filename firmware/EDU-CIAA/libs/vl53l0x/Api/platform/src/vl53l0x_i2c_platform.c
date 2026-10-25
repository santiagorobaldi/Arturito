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

/* EDU-CIAA backend: original ST signatures, fixed HAL bus A (I2C0).
 * Address uses ST's 8-bit convention. The application serializes access.
 * Blocking transfers have no timeout if the peripheral stops progressing.
 */
#include "vl53l0x_i2c_platform.h"
#include "hal_i2c.h"
#include <string.h>

#define STATUS_OK 0
#define STATUS_FAIL 1

int32_t VL53L0X_comms_initialise(uint8_t comms_type, uint16_t comms_speed_khz) //Falta enlazar esto con el init del driver
{
   return STATUS_OK;
}

int32_t VL53L0X_comms_close(void)
{
    /* The application owns the shared bus; closing this sensor leaves it active. */
    return STATUS_OK;
}

int32_t VL53L0X_write_multi(uint8_t address, uint8_t index, uint8_t *pdata,
                           int32_t count)
{
    uint8_t buffer[COMMS_BUFFER_SIZE];
    if (pdata == NULL || count <= 0 || count >= COMMS_BUFFER_SIZE ||
        address == 0 || (address & 1U) != 0)
        return STATUS_FAIL;

    buffer[0] = index;
    memcpy(&buffer[1], pdata, (size_t)count);
    return hal_i2c_write(HAL_I2C_BUS_A, address >> 1, buffer, (int)(count + 1))
               ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata,
                          int32_t count)
{
    if (pdata == NULL || count <= 0 || count >= COMMS_BUFFER_SIZE ||
        address == 0 || (address & 1U) != 0)
        return STATUS_FAIL;

    /* HAL preserves the repeated START between register selection and reading. */
    return hal_i2c_write_read(HAL_I2C_BUS_A, address >> 1,
                              &index, 1, pdata, (int)count)
               ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    int32_t status = STATUS_OK;
    const int32_t cbyte_count = 1;

    status = VL53L0X_write_multi(address, index, &data, cbyte_count);

    return status;

}


int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    int32_t status = STATUS_OK;

    uint8_t  buffer[BYTES_PER_WORD];

    // Split 16-bit word into MS and LS uint8_t
    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data &  0x00FF);

    status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);

    return status;

}


int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    int32_t status = STATUS_OK;
    uint8_t  buffer[BYTES_PER_DWORD];

    // Split 32-bit word into MS ... LS bytes
    buffer[0] = (uint8_t) (data >> 24);
    buffer[1] = (uint8_t)((data &  0x00FF0000) >> 16);
    buffer[2] = (uint8_t)((data &  0x0000FF00) >> 8);
    buffer[3] = (uint8_t) (data &  0x000000FF);

    status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_DWORD);

    return status;

}


int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
    int32_t status = STATUS_OK;
    int32_t cbyte_count = 1;

    status = VL53L0X_read_multi(address, index, pdata, cbyte_count);

    return status;

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


/* These optional ST hooks are not used by the ranging core in this package.
 * No GPIO/power wiring or accurate timer backend has been selected. Return
 * failure explicitly instead of pretending that an operation was performed.
 */
int32_t VL53L0X_platform_wait_us(int32_t wait_us)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // int32_t status = STATUS_OK;
    //     float wait_ms = (float)wait_us/1000.0f;
    // 
    //     /*
    //      * Use windows event handling to perform non-blocking wait.
    //      */
    //     HANDLE hEvent = CreateEvent(0, TRUE, FALSE, 0);
    //     WaitForSingleObject(hEvent, (int)(wait_ms + 0.5f));
    // 
    // #ifdef VL53L0X_LOG_ENABLE
    //     trace_i2c("Wait us : %6d\n", wait_us);
    // #endif
    // 
    //     return status;

    /* Comportamiento provisional EDU-CIAA. */
    return wait_us == 0 ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_wait_ms(int32_t wait_ms)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // int32_t status = STATUS_OK;
    // 
    //     /*
    //      * Use windows event handling to perform non-blocking wait.
    //      */
    //     HANDLE hEvent = CreateEvent(0, TRUE, FALSE, 0);
    //     WaitForSingleObject(hEvent, wait_ms);
    // 
    // #ifdef VL53L0X_LOG_ENABLE
    //     trace_i2c("Wait ms : %6d\n", wait_ms);
    // #endif
    // 
    //     return status;

    /* Comportamiento provisional EDU-CIAA. */
    return wait_ms == 0 ? STATUS_OK : STATUS_FAIL;
}

int32_t VL53L0X_set_gpio(uint8_t level)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // int32_t status = STATUS_OK;
    //     //status = VL53L0X_set_gpio_sv(level);
    // #ifdef VL53L0X_LOG_ENABLE
    //     trace_i2c("// Set GPIO = %d;\n", level);
    // #endif
    // 
    //     return status;

    /* Comportamiento provisional EDU-CIAA. */
    (void)level;
    return STATUS_FAIL;
}

int32_t VL53L0X_get_gpio(uint8_t *plevel)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // int32_t status = STATUS_OK;
    // #ifdef VL53L0X_LOG_ENABLE
    //     trace_i2c("// Get GPIO = %d;\n", *plevel);
    // #endif
    //     return status;

    /* Comportamiento provisional EDU-CIAA. */
    if (plevel != NULL) *plevel = 0;
    return STATUS_FAIL;
}

int32_t VL53L0X_release_gpio(void)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // int32_t status = STATUS_OK;
    // #ifdef VL53L0X_LOG_ENABLE
    //     trace_i2c("// Releasing force on GPIO\n");
    // #endif
    //     return status;

    /* Comportamiento provisional EDU-CIAA. */
    return STATUS_FAIL;
}

int32_t VL53L0X_cycle_power(void)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // int32_t status = STATUS_OK;
    // #ifdef VL53L0X_LOG_ENABLE
    //     trace_i2c("// cycle sensor power\n");
    // #endif
    //     status = RANGING_SENSOR_COMMS_Cycle_Sensor_Power();
    // 
    // 	if(status != STATUS_OK)
    //     {
    //         RANGING_SENSOR_COMMS_Get_Error_Text(debug_string);
    //     }
    // 
    // 	return status;

    /* Comportamiento provisional EDU-CIAA. */
    return STATUS_FAIL;
}

int32_t VL53L0X_get_timer_frequency(int32_t *ptimer_freq_hz)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // *ptimer_freq_hz = 0;
    //        return STATUS_FAIL;

    /* Comportamiento provisional EDU-CIAA. */
    if (ptimer_freq_hz != NULL) *ptimer_freq_hz = 0;
    return STATUS_FAIL;
}

int32_t VL53L0X_get_timer_value(int32_t *ptimer_count)
{
    /* NO SOPORTADA POR AHORA EN EDU-CIAA.
     * Codigo original ST a continuacion, solo como referencia; no se ejecuta.
     */
    // *ptimer_count = 0;
    //        return STATUS_FAIL;

    /* Comportamiento provisional EDU-CIAA. */
    if (ptimer_count != NULL) *ptimer_count = 0;
    return STATUS_FAIL;
}
