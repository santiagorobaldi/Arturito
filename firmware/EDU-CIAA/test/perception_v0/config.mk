# Ensayo de percepcion V0: MPU6050 + VL53L0X + orientacion + serie.
VERBOSE=n
OPT=g
USE_NANO=n
SEMIHOST=n
USE_FPU=y
DEFINES += MPU60X0_DEBUG
DEFINES += BOARD_EDUCIAA

USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=y
USE_VL53L0X=y
FREERTOS_HEAP_TYPE=4
LOAD_INRAM=n

INCLUDES += -I../shared/config
INCLUDES += -Irover/config
INCLUDES += -I../shared/layer1_hal
INCLUDES += -I../shared/layer2_drivers
INCLUDES += -I../shared/layer3_app
INCLUDES += -I../shared/tests/perception_v0
INCLUDES += -Itest/perception_v0/inc

SRC += ../shared/layer2_drivers/driver_mpu6050.c
SRC += ../shared/layer2_drivers/driver_vl53l0x.c
SRC += ../shared/tests/perception_v0/perception_v0.c

# Adaptaciones de placa compartidas con la entrada heredada rover.
SRC += rover/src/layer1_hal/hal_i2c_educiaa.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_mpu6050_educiaa.c
SRC += rover/src/layer1_hal/hal_vl53l0x_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c

# Make descubre automaticamente este main para el programa perception_v0.
# La seleccion heredada rover lo incorpora explicitamente.
ifeq ($(PROGRAM_NAME),rover)
SRC += test/perception_v0/src/main.c
endif
