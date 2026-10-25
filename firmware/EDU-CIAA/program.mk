# El modo se elige con APP_MODE; por defecto queda activa la prueba del reloj.
PROGRAM_PATH =
PROGRAM_NAME = rover

# Elegir con `make APP_MODE=clock_test`, `make APP_MODE=tof_test` o `make APP_MODE=rover`.
APP_MODE ?= clock_test

ifeq ($(APP_MODE),clock_test)
# La configuracion del rover selecciona el adaptador y las fuentes del test.
else ifeq ($(APP_MODE),tof_test)
# La configuracion del rover selecciona el adaptador y las fuentes del ToF.
else ifeq ($(APP_MODE),telemetry_test)
# La configuracion del rover selecciona el ensayo de telemetria por UART.
else ifeq ($(APP_MODE),imu_test)
# La configuracion del rover selecciona el ensayo de angulo por telemetria.
else ifeq ($(APP_MODE),rover)
# La configuracion del rover selecciona las fuentes de la aplicacion.
else ifeq ($(APP_MODE),encoder_test)
# La configuracion del rover selecciona las fuentes del test de encoders.
else
$(error APP_MODE debe ser clock_test, tof_test, telemetry_test, imu_test, encoder_test o rover)
endif
