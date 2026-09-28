# Compile options
VERBOSE=n
OPT=g
USE_NANO=n
SEMIHOST=n
USE_FPU=y

# Libraries
USE_LPCOPEN=y
USE_SAPI=y
USE_FREERTOS=y
FREERTOS_HEAP_TYPE=5
LOAD_INRAM=n

# --- Integración de la Arquitectura Compartida (Arturito) ---

# 1. Rutas de las cabeceras (.h) compartidas
INCLUDES += -I../shared/config
INCLUDES += -I../shared/layer1_hal
INCLUDES += -I../shared/layer2_drivers
INCLUDES += -I../shared/layer3_robotics
INCLUDES += -I../shared/layer4_app

# 2. Archivos fuente (.c) compartidos a compilar
SRC += $(wildcard ../shared/layer2_drivers/*.c)
SRC += $(wildcard ../shared/layer3_robotics/*.c)
SRC += $(wildcard ../shared/layer4_app/*.c)

# 3. HAL de EDU-CIAA (C++ por usar namespace)
CXXSRC += $(PROGRAM_PATH_AND_NAME)/src/layer1_hal/hal_sensors_educiaa.cpp