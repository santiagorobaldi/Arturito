ifeq ($(USE_VL53L0X),y)
VL53L0X_BASE=libs/vl53l0x
INCLUDES += -I$(VL53L0X_BASE)/Api/platform/inc
INCLUDES += -I$(VL53L0X_BASE)/Api/core/inc
SRC += $(wildcard $(VL53L0X_BASE)/Api/core/src/*.c)
SRC += $(VL53L0X_BASE)/Api/platform/src/vl53l0x_platform.c
SRC += $(VL53L0X_BASE)/Api/platform/src/vl53l0x_i2c_platform.c
endif
