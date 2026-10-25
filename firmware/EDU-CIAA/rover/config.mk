 VERBOSE=n
 OPT=g
 USE_NANO=n
 SEMIHOST=n
 USE_FPU=y
 DEFINES += BOARD_EDUCIAA

 USE_LPCOPEN=y
 USE_SAPI=y
 USE_VL53L0X=y
 LOAD_INRAM=n

 INCLUDES += -Irover/config
 INCLUDES += -I../shared/layer1_hal
 INCLUDES += -I../shared/layer2_drivers
 INCLUDES += -I../shared/layer3_app
+INCLUDES += -I../shared/test/layer3_app

 SRC += ../shared/layer2_drivers/driver_mpu6050.c
 SRC += ../shared/layer2_drivers/driver_vl53l0x.c
+SRC += ../shared/layer3_app/main.c
 SRC += ../shared/layer3_app/angle.c
 SRC += ../shared/layer3_app/distance.c
+SRC += $(wildcard ../shared/test/layer3_app/*.c)

 SRC += rover/src/layer1_hal/hal_i2c_educiaa.c
 SRC += rover/src/layer1_hal/hal_time_educiaa.c
 SRC += rover/src/layer1_hal/hal_mpu6050_educiaa.c
 SRC += rover/src/layer1_hal/hal_vl53l0x_educiaa.c
 SRC += rover/src/layer1_hal/hal_telemetry_educiaa.c