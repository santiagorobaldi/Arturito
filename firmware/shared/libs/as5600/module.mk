ifeq ($(USE_AS5600),y)
AS5600_BASE=libs/as5600
INCLUDES += -I$(AS5600_BASE)/source
SRC += $(AS5600_BASE)/source/driver_as5600.c

endif
