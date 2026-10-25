#include "hal_i2c.h"
#include "sapi.h"
#include "educiaa_board_config.h"
#include "i2c_18xx_43xx.h"

static bool bus_a_initialized = false;
static bool bus_b_initialized = false;

bool hal_i2c_init(hal_i2c_bus_t bus){
    switch (bus) {
    case HAL_I2C_BUS_A:
        Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
        Chip_I2C_Init(I2C0);
        Chip_I2C_SetClockRate(I2C0, BOARD_I2C_A_CLOCK_HZ);
        if (Chip_I2C_SetMasterEventHandler(I2C0, Chip_I2C_EventHandlerPolling) != 0) {
            bus_a_initialized = true;
            return true;
        }else 
            return false;
    case HAL_I2C_BUS_B: {
        const uint16_t mode =
            SCU_MODE_INACT |
            SCU_MODE_INBUFF_EN |
            SCU_MODE_ZIF_DIS |
            SCU_MODE_FUNC1;

        
        Chip_SCU_PinMuxSet(
            BOARD_I2C_B_SDA_SCU_PORT,
            BOARD_I2C_B_SDA_SCU_PIN,
            mode
        );

       
        Chip_SCU_PinMuxSet(
            BOARD_I2C_B_SCL_SCU_PORT,
            BOARD_I2C_B_SCL_SCU_PIN,
            mode
        );
        Chip_I2C_Init(I2C1);
        Chip_I2C_SetClockRate(I2C1, BOARD_I2C_B_CLOCK_HZ);
        if (Chip_I2C_SetMasterEventHandler(I2C1, Chip_I2C_EventHandlerPolling) != 0) {
            bus_b_initialized = true;
            return true;
        } else {
            return false;
        }
    }
    default:
        return false;
    }
}

bool hal_i2c_is_initialized(hal_i2c_bus_t bus){
    switch (bus) {
    case HAL_I2C_BUS_A:
        return bus_a_initialized;
    case HAL_I2C_BUS_B:
        return bus_b_initialized;
    default:
        return false;
    }
}

bool hal_i2c_write(hal_i2c_bus_t bus, uint8_t addr  , const uint8_t *data, int len){
    if (len <= 0 || data == NULL || (bus != HAL_I2C_BUS_A && bus != HAL_I2C_BUS_B)) {
        return false; 
    }
    I2C_XFER_T xfer = {0};
    xfer.txBuff = data;
    xfer.txSz = len;
    xfer.rxBuff = NULL;
    xfer.rxSz = 0;
    xfer.slaveAddr = addr;

    return Chip_I2C_MasterTransfer(bus == HAL_I2C_BUS_A ? I2C0 : I2C1, &xfer) == I2C_STATUS_DONE;
}

bool hal_i2c_read(hal_i2c_bus_t bus, uint8_t addr, uint8_t *data, int len){
    if (len <= 0 || data == NULL || (bus != HAL_I2C_BUS_A && bus != HAL_I2C_BUS_B)) {
        return false; 
    }
    I2C_XFER_T xfer = {0};
    xfer.txBuff = NULL;
    xfer.txSz = 0;
    xfer.rxBuff = data;
    xfer.rxSz = len;
    xfer.slaveAddr = addr;

    return Chip_I2C_MasterTransfer(bus == HAL_I2C_BUS_A ? I2C0 : I2C1, &xfer) == I2C_STATUS_DONE;
}


bool hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t addr, const uint8_t *data_out, int len_out, uint8_t *data_in, int len_in) {
    if (len_out <= 0 || len_in <= 0 || data_out == NULL || data_in == NULL ||
        (bus != HAL_I2C_BUS_A && bus != HAL_I2C_BUS_B)) {
        return false;
    }
    I2C_XFER_T xfer = {0};
    xfer.txBuff = data_out;
    xfer.txSz = len_out;
    xfer.rxBuff = data_in;
    xfer.rxSz = len_in;
    xfer.slaveAddr = addr;

    return Chip_I2C_MasterTransfer(bus == HAL_I2C_BUS_A ? I2C0 : I2C1, &xfer) == I2C_STATUS_DONE;
}

