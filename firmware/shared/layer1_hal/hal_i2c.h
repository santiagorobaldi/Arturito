#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    HAL_I2C_BUS_A,
    HAL_I2C_BUS_B
} hal_i2c_bus_t;

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Inicializa el bus I2C enviado por parametro. Devuelve true si la inicialización fue exitosa, false en caso contrario.
 */
bool hal_i2c_init(hal_i2c_bus_t bus);
/* Escribe datos en el bus I2C */
bool hal_i2c_write(hal_i2c_bus_t bus, uint8_t addr  , const uint8_t *data, int len);
/* Lee datos del bus I2C */
bool hal_i2c_read(hal_i2c_bus_t bus, uint8_t addr, uint8_t *data, int len);
/* Escribe y lee datos en el bus I2C */
bool hal_i2c_write_read(hal_i2c_bus_t bus, uint8_t addr, const uint8_t *data_out, int len_out, uint8_t *data_in, int len_in);
/* Verifica si el bus I2C está inicializado */
bool hal_i2c_is_initialized(hal_i2c_bus_t bus);

#ifdef __cplusplus
}
#endif
#endif /* HAL_I2C_H */

