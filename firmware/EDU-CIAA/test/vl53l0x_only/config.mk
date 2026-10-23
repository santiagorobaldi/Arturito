# Ensayo aislado de VL53L0X mediante el driver compartido.
VERBOSE=n
OPT=g
USE_NANO=n
SEMIHOST=n
USE_FPU=y
DEFINES += BOARD_EDUCIAA

USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=y
USE_VL53L0X=y
FREERTOS_HEAP_TYPE=4
LOAD_INRAM=n

INCLUDES += -I../shared/layer1_hal
INCLUDES += -I../shared/layer2_drivers
INCLUDES += -Irover/config
# Reutilizar la configuracion FreeRTOS ya usada en esta placa.
INCLUDES += -Itest/perception_v0/inc

SRC += ../shared/layer2_drivers/driver_vl53l0x.c
SRC += rover/src/layer1_hal/hal_i2c_educiaa.c
SRC += rover/src/layer1_hal/hal_vl53l0x_educiaa.c
