#include "hal_i2c.h"
#include "hal_time.h"
#include "sapi.h"
#include "chip.h"
#include "educiaa_board_config.h"
#include "i2c_18xx_43xx.h"

#define I2C_TIMEOUT_US 10000U
#define I2C_CLEAR_HALF_PERIOD_US 5U
#define I2C_CLEAR_CLOCK_TIMEOUT_US 1000U
#define I2C_CLEAR_MAX_PULSES 9U

static bool bus_a_initialized = false;
static bool bus_b_initialized = false;

bool hal_i2c_init(hal_i2c_bus_t bus){
    if (!hal_time_init()) {
        return false;
    }
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
        /* Mantener activo el filtro de glitches de SDA y SCL. */
        const uint16_t mode =
            SCU_MODE_INACT |
            SCU_MODE_INBUFF_EN |
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

static void recovery_delay(void)
{
    const uint32_t start = hal_time_us();
    while (hal_time_elapsed_us(start) < I2C_CLEAR_HALF_PERIOD_US) {}
}

static bool release_scl(void)
{
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, BOARD_I2C_B_GPIO_PORT,
                             BOARD_I2C_B_SCL_GPIO_PIN);
    const uint32_t start = hal_time_us();
    while (!Chip_GPIO_GetPinState(LPC_GPIO_PORT, BOARD_I2C_B_GPIO_PORT,
                                 BOARD_I2C_B_SCL_GPIO_PIN)) {
        if (hal_time_elapsed_us(start) >= I2C_CLEAR_CLOCK_TIMEOUT_US) {
            return false;
        }
    }
    return true;
}

static void clear_bus_b(void)
{
    const uint8_t port = BOARD_I2C_B_GPIO_PORT;
    const uint8_t sda = BOARD_I2C_B_SDA_GPIO_PIN;
    const uint8_t scl = BOARD_I2C_B_SCL_GPIO_PIN;
    const uint16_t gpio_mode =
        SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC4;

    /* Open drain por direccion: salida baja o entrada con pull-up externo. */
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, port, sda);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, port, scl);
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, port, sda);
    Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT, port, scl);
    Chip_SCU_PinMuxSet(BOARD_I2C_B_SDA_SCU_PORT, BOARD_I2C_B_SDA_SCU_PIN, gpio_mode);
    Chip_SCU_PinMuxSet(BOARD_I2C_B_SCL_SCU_PORT, BOARD_I2C_B_SCL_SCU_PIN, gpio_mode);

    if (!release_scl()) {
        return;
    }
    recovery_delay();
    if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, port, sda)) {
        return;
    }

    for (unsigned int pulse = 0; pulse < I2C_CLEAR_MAX_PULSES; ++pulse) {
        Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, port, scl);
        recovery_delay();
        if (!release_scl()) {
            return;
        }
        recovery_delay();
        if (Chip_GPIO_GetPinState(LPC_GPIO_PORT, port, sda)) {
            /* STOP: soltar SDA mientras SCL esta alto. */
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, port, scl);
            recovery_delay();
            Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, port, sda);
            recovery_delay();
            release_scl();
            recovery_delay();
            Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, port, sda);
            recovery_delay();
            return;
        }
    }
}

static bool transfer(hal_i2c_bus_t bus, I2C_XFER_T *xfer)
{
    if (!hal_i2c_is_initialized(bus)) {
        return false;
    }
    const bool bus_a = bus == HAL_I2C_BUS_A;
    const I2C_ID_T id = bus_a ? I2C0 : I2C1;
    const I2C_STATUS_T status =
        Chip_I2C_MasterTransferTimed(id, xfer, hal_time_us, I2C_TIMEOUT_US);
    if (status == I2C_STATUS_TIMEOUT) {
        /* Resetear con el reloj del I2C activo antes de soltar los pines. */
        Chip_RGU_TriggerReset(bus_a ? RGU_I2C0_RST : RGU_I2C1_RST);
        Chip_I2C_DeInit(id);
        if (bus_a) {
            bus_a_initialized = false;
        } else {
            bus_b_initialized = false;
            clear_bus_b();
        }
        hal_i2c_init(bus);
    }
    return status == I2C_STATUS_DONE;
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

    return transfer(bus, &xfer);
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

    return transfer(bus, &xfer);
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

    return transfer(bus, &xfer);
}

