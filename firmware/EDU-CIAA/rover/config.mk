VERBOSE=n
OPT=g
USE_NANO=n
SEMIHOST=n
USE_FPU=y
DEFINES += BOARD_EDUCIAA

USE_LPCOPEN=y
USE_SAPI=y
LOAD_INRAM=n

VL53L0X_ROOT = ../shared/libs/VL53L0X_1.0.4

INCLUDES += -Irover/config
INCLUDES += -I../shared/layer1_hal
INCLUDES += -I../shared/layer2_drivers
INCLUDES += -I../shared/layer3_app
INCLUDES += -I../shared/layer3_app/localization
INCLUDES += -I../shared/layer3_app/sensing
INCLUDES += -I../shared/layer3_app/telemtry

ifeq ($(APP_MODE),clock_test)
INCLUDES += -I../shared/test/hal_time
SRC += ../shared/test/hal_time/test_app.c
SRC += ../shared/test/hal_time/test_hal_time.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c
else ifeq ($(APP_MODE),encoder_test)
INCLUDES += -I../shared/libs/as5600/source
SRC += ../shared/test/encoder/test_app.c
SRC += ../shared/layer2_drivers/driver_encoder.c
SRC += ../shared/libs/as5600/source/driver_as5600.c
SRC += rover/src/layer1_hal/hal_i2c_educiaa.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c
else ifeq ($(APP_MODE),tof_test)
INCLUDES += -I../shared/test/tof
INCLUDES += -I$(VL53L0X_ROOT)/Api/core/inc
INCLUDES += -I$(VL53L0X_ROOT)/Api/platform/inc
SRC += ../shared/test/tof/test_app.c
SRC += ../shared/test/tof/test_tof.c
SRC += ../shared/layer3_app/sensing/distance.c
SRC += ../shared/layer2_drivers/driver_vl53l0x.c
SRC += rover/src/layer1_hal/hal_i2c_educiaa.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c
SRC += $(wildcard $(VL53L0X_ROOT)/Api/core/src/*.c)
SRC += $(VL53L0X_ROOT)/Api/platform/src/vl53l0x_platform.c
SRC += $(VL53L0X_ROOT)/Api/platform/src/vl53l0x_i2c_platform.c
else ifeq ($(APP_MODE),telemetry_test)
INCLUDES += -I../shared/test/telemetry
SRC += ../shared/test/telemetry/test_telemetry.c
SRC += ../shared/layer3_app/telemtry/telemetry_app.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c
else ifeq ($(APP_MODE),imu_test)
SRC += ../shared/test/imu/test_imu.c
SRC += ../shared/layer3_app/sensing/angle.c
SRC += ../shared/layer2_drivers/driver_mpu6050.c
SRC += ../shared/layer3_app/telemtry/telemetry_app.c
SRC += rover/src/layer1_hal/hal_mpu6050_educiaa.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c
else
USE_AS5600=y
INCLUDES += -I$(VL53L0X_ROOT)/Api/core/inc
INCLUDES += -I$(VL53L0X_ROOT)/Api/platform/inc
INCLUDES += -I../shared/libs/as5600/source
SRC += ../shared/layer2_drivers/driver_mpu6050.c
SRC += ../shared/layer2_drivers/driver_vl53l0x.c
SRC += ../shared/layer2_drivers/driver_encoder.c
SRC += $(wildcard ../shared/layer3_app/localization/*.c)
SRC += $(wildcard ../shared/layer3_app/sensing/*.c)
SRC += ../shared/layer3_app/main.c
SRC += ../shared/layer3_app/telemtry/telemetry_app.c
SRC += ../shared/libs/as5600/source/driver_as5600.c
SRC += rover/src/layer1_hal/hal_i2c_educiaa.c
SRC += rover/src/layer1_hal/hal_time_educiaa.c
SRC += rover/src/layer1_hal/hal_mpu6050_educiaa.c
SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c
SRC += $(wildcard $(VL53L0X_ROOT)/Api/core/src/*.c)
SRC += $(VL53L0X_ROOT)/Api/platform/src/vl53l0x_platform.c
SRC += $(VL53L0X_ROOT)/Api/platform/src/vl53l0x_i2c_platform.c
endif
