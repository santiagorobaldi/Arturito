#include "hal_i2c.h"
#include <Wire.h>
#include "esp32_board_config.h"


bool hal_i2c_init(hal_i2c_bus_t bus) {
    switch (bus) {
    case HAL_I2C_BUS_A:
        
        return Wire.begin(
            BOARD_I2C_A_SDA_GPIO,
            BOARD_I2C_A_SCL_GPIO,
            BOARD_I2C_A_CLOCK_HZ
        );

    case HAL_I2C_BUS_B:
        
        return Wire1.begin(
            BOARD_I2C_B_SDA_GPIO,
            BOARD_I2C_B_SCL_GPIO,
            BOARD_I2C_B_CLOCK_HZ
        );

    default:
        return false;
    }
}

bool hal_i2c_write(hal_i2c_bus_t bus, uint8_t addr, const uint8_t *data, int len) {
    if (len <= 0 || len > I2C_BUFFER_LENGTH || data == NULL || (bus != HAL_I2C_BUS_A && bus != HAL_I2C_BUS_B)) {
        return false; 
    }
    TwoWire *wire = (bus == HAL_I2C_BUS_A) ? &Wire : &Wire1;

    wire->beginTransmission(addr);
    if (wire->write(data, (size_t) len) != (size_t) len) {
        wire->flush(); // Descarta bytes parciales antes de cerrar la operacion.
        wire->endTransmission(true);
        return false;
    }
    uint8_t result = wire->endTransmission(true);

    return (result == 0);
}

bool hal_i2c_read (hal_i2c_bus_t bus, uint8_t addr, uint8_t *data, int len) {
    if (len <= 0 || len > I2C_BUFFER_LENGTH || data == NULL || (bus != HAL_I2C_BUS_A && bus != HAL_I2C_BUS_B)) {
        return false; 
    }
    TwoWire *wire = (bus == HAL_I2C_BUS_A) ? &Wire : &Wire1;

    size_t bytesRead = wire->requestFrom(addr, (size_t) len, true);
    if (bytesRead != (size_t) len) {
        return false;
    }

    for (int i = 0; i < len; ++i) {
        data[i] = wire->read();
    }

    return true;

}

bool hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t addr, const uint8_t *data_out, int len_out, uint8_t *data_in, int len_in) {
    if (len_out <= 0 || len_in <= 0 || len_out > I2C_BUFFER_LENGTH || len_in > I2C_BUFFER_LENGTH || data_out == NULL || data_in == NULL || (bus != HAL_I2C_BUS_A && bus != HAL_I2C_BUS_B)) {
        return false; 
    }
    TwoWire *wire = (bus == HAL_I2C_BUS_A) ? &Wire : &Wire1;

    wire->beginTransmission(addr);
    if (wire->write(data_out, (size_t) len_out) != (size_t) len_out) {
        wire->flush(); // Descarta bytes parciales antes de cerrar la operacion.
        wire->endTransmission(true);
        return false;
    }

    uint8_t result = wire->endTransmission(false); 

    if (result != 0) {
        return false;
    }

    size_t bytesRead = wire->requestFrom(addr, (size_t) len_in, true);
    if (bytesRead != (size_t) len_in) {
        return false;
    }

    for (int i = 0; i < len_in; ++i) {
        data_in[i] = wire->read();
    }

    return true;
}

