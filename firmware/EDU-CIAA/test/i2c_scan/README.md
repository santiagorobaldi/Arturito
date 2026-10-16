# Diagnostico I2C para EDU-CIAA

Este programa escanea las direcciones I2C estandar y lee `WHO_AM_I` si encuentra un dispositivo en `0x68` o `0x69`. Usa el estado de transferencia del LPC43xx directamente: en esta version de sAPI, el valor booleano de `i2cRead()` no distingue de forma fiable un NACK.

## Compilar y cargar

Desde la carpeta `edu-ciaa`:

```sh
make PROGRAM_PATH=test PROGRAM_NAME=i2c_scan all
make PROGRAM_PATH=test PROGRAM_NAME=i2c_scan download
```

Abrir el puerto serie de la placa a 115200 baudios. El escaneo se repite cada 3 segundos. Este programa reemplaza temporalmente el firmware del rover mientras este cargado.

## Conexion

- MPU `VCC` a 3,3 V y `GND` a GND de la EDU-CIAA.
- MPU `SDA` y `SCL` a las lineas I2C0 SDA/SCL de la EDU-CIAA.
- Verificar pull-ups de SDA y SCL a 3,3 V; algunos modulos ya las incorporan.
- Con `AD0` bajo, la direccion esperada es `0x68`; con `AD0` alto, es `0x69`.

El repositorio configura el periferico I2C0, pero no incluye el esquema del conector para identificar sus cavidades fisicas. Usar el pinout de la revision concreta de la EDU-CIAA.

## Interpretacion

- Sin direcciones: revisar alimentacion, masa comun, cableado, pull-ups y que se usen los pines I2C0.
- `0x68` o `0x69` con `WHO_AM_I = 0x72`: hay respuesta I2C, pero el chip no reporta el ID clasico `0x68`; anotar el valor y verificar el modelo del modulo.
- Direccion detectada pero falla la lectura de `WHO_AM_I`: revisar el sensor y la transaccion; el estado I2C hexadecimal ayuda a distinguir el NACK.
- Si la ejecucion se detiene durante el escaneo, puede haber una linea del bus trabada en bajo o un problema electrico.