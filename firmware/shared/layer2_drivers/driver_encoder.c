#include "driver_encoder.h"
#include "driver_as5600.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "hal_telemetry.h"
#include <string.h>


#define TWO_PI 6.28318530718f

static uint8_t adaptador_iic_init_busA(void);
static uint8_t adaptador_iic_init_busB(void);
static uint8_t adaptador_iic_deinit_busA(void);
static uint8_t adaptador_iic_deinit_busB(void);
static uint8_t adaptador_iic_read_busA(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
static uint8_t adaptador_iic_read_busB(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
static uint8_t adaptador_iic_write_busA(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
static uint8_t adaptador_iic_write_busB(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
static void adaptador_delay_ms(uint32_t ms);
static void adaptador_debug_print(const char *const fmt, ...);

static as5600_handle_t handleEncIzq = {
    *adaptador_iic_init_busA,
    *adaptador_iic_deinit_busA,
    *adaptador_iic_read_busA,
    *adaptador_iic_write_busA,
    *adaptador_delay_ms,
    *adaptador_debug_print,
    0
};

static as5600_handle_t handleEncDer = {
    *adaptador_iic_init_busB,
    *adaptador_iic_deinit_busB,
    *adaptador_iic_read_busB,
    *adaptador_iic_write_busB,
    *adaptador_delay_ms,
    *adaptador_debug_print,
    0
};

static uint8_t adaptador_iic_init_busA () {

    return hal_i2c_is_initialized(HAL_I2C_BUS_A) ? 0 : 1;
}

static uint8_t adaptador_iic_init_busB () {
    
    return hal_i2c_is_initialized(HAL_I2C_BUS_B) ? 0 : 1;
}

static uint8_t adaptador_iic_deinit_busA () { //No debe poder desinicializarse el bus A, ya que es compartido con otros dispositivos
        return 0;
}

static uint8_t adaptador_iic_deinit_busB () { //No debe poder desinicializarse el bus B, ya que es compartido con otros dispositivos
        return 0;
}


static uint8_t adaptador_iic_read_busA (uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  
        return hal_i2c_write_read(HAL_I2C_BUS_A, (uint8_t)(addr >> 1), &reg, 1, buf, len) ? 0 : 1;

}

static uint8_t adaptador_iic_read_busB (uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  
        return hal_i2c_write_read(HAL_I2C_BUS_B, (uint8_t)(addr >> 1), &reg, 1, buf, len)? 0 : 1;

}

static uint8_t adaptador_iic_write_busA (uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  
    uint8_t buffer[len+1];
    buffer[0]=reg;
    memcpy(&buffer[1], buf, len);
  
    return hal_i2c_write(HAL_I2C_BUS_A, (uint8_t)(addr >> 1), buffer, len+1) ? 0 : 1;
}

static uint8_t adaptador_iic_write_busB (uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
    uint8_t buffer[len+1];
    buffer[0]=reg;
    memcpy(&buffer[1], buf, len);
  
    return hal_i2c_write(HAL_I2C_BUS_B, (uint8_t)(addr >> 1), buffer, len+1) ? 0 : 1;
}



static void adaptador_delay_ms (uint32_t ms) {
    hal_delay(ms);
}

static void adaptador_debug_print (const char *const fmt, ...) {
    hal_telemetry_write(fmt);
}

encoder_status_t encoder_init(encoder_t encoder){
        switch (encoder)
        {
            case ENC_IZQ: {
                uint8_t status = as5600_init(&handleEncIzq);
                switch (status)
                {
                    case 0:
                        return ENCODER_OK;
                    case 1:
                        return ENCODER_ERROR_COMMUNICATION;
                    case 2:
                        return ENCODER_ERROR_ARGUMENT;
                    case 3:
                        return ENCODER_ERROR_INTERNAL_API;
                    default:
                        return ENCODER_ERROR_INTERNAL_API;
                }
            }
            case ENC_DER: {
                uint8_t status = as5600_init(&handleEncDer);
                switch (status)
                {
                    case 0:
                        return ENCODER_OK;
                    case 1:
                        return ENCODER_ERROR_COMMUNICATION;
                    case 2:
                        return ENCODER_ERROR_ARGUMENT;
                    case 3:
                        return ENCODER_ERROR_INTERNAL_API;
                    default:
                        return ENCODER_ERROR_INTERNAL_API;
                }
            }
            default:
                return ENCODER_ERROR_ARGUMENT;
        }
}

encoder_status_t encoder_get_angle_rad(encoder_t encoder, float *value){
    if (value == NULL) {
        return ENCODER_ERROR_ARGUMENT;
    }
    uint16_t count;
     switch (encoder)
        {
            case ENC_IZQ: {
                uint8_t status = as5600_get_raw_angle(&handleEncIzq, &count);
                switch (status)
                {
                    case 0:{
                        *value=count*TWO_PI/4096.0f; //Convertimos el valor del encoder a radianes
                        return ENCODER_OK;
                    }
                    case 1:
                        return ENCODER_ERROR_COMMUNICATION;
                    case 2:
                        return ENCODER_ERROR_INTERNAL_API;
                    case 3:
                        return ENCODER_ERROR_INTERNAL_API;
                    default:        
                        return ENCODER_ERROR_INTERNAL_API;
                }
                
            }
            case ENC_DER: {
                uint8_t status = as5600_get_raw_angle(&handleEncDer, &count);
                switch (status)
                {
                    case 0:{
                        *value=count*TWO_PI/4096.0f; //Convertimos el valor del encoder a radianes
                        return ENCODER_OK;
                    }
                    case 1:
                        return ENCODER_ERROR_COMMUNICATION;
                    case 2:
                        return ENCODER_ERROR_INTERNAL_API;
                    case 3:
                        return ENCODER_ERROR_INTERNAL_API;
                    default:
                        return ENCODER_ERROR_INTERNAL_API;
                }
            }
               
            default:
                return ENCODER_ERROR_ARGUMENT;
        }
    
}



