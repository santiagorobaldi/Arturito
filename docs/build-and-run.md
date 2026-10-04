# Compilación y ejecución

## Preparación

Usar un checkout completo con `shared`, `ESP32` y `EDU-CIAA` como carpetas hermanas. Respetar mayúsculas: algunos textos y rutas del workspace usan `esp32`, `edu-ciaa` y `processing`, pero los nombres versionados son `ESP32`, `EDU-CIAA` y `Processing`.

ESP32 requiere PlatformIO con la plataforma espressif32 y Arduino, además de un puerto serie accesible para carga. EDU-CIAA requiere GNU Make, herramientas `arm-none-eabi-*` y utilidades de shell compatibles con las recetas del Makefile; para carga utiliza OpenOCD con `scripts/openocd/lpc4337.cfg`. En Windows debe usarse un entorno que proporcione esas herramientas. Las bibliotecas de EDU-CIAA están incluidas en `libs/`.

Esta revisión inspeccionó las configuraciones; no ejecutó compilaciones ni cargas. Los antecedentes registrados por el equipo se distinguen en [Estado](development-status.md).

## ESP32

Desde `ESP32/`:

```sh
pio run -e esp32dev
pio run -e imu_only
pio run -e tof_only
```

`esp32dev` es el entorno predeterminado. Los diagnósticos aíslan cada sensor; no usan las tres tareas del runtime completo. El entorno base comparte configuración y no es el perfil de aplicación que se recomienda compilar.

Para cargar el entorno elegido en una placa conectada:

```sh
pio run -e esp32dev -t upload
pio device monitor -b 115200
```

Para cargar un diagnóstico sustituir `esp32dev` por `imu_only` o `tof_only`. Cada carga reemplaza el firmware que estaba en la placa. Volver a cargar `esp32dev` al terminar. El puerto de carga no está fijado en `platformio.ini`; comprobar la selección de PlatformIO.

## EDU-CIAA

Desde la raíz del repositorio:

```sh
make -C EDU-CIAA all
```

`program.mk` selecciona `rover`. El build activa LPCOpen, sAPI, FreeRTOS y heap tipo 4; agrega drivers/runtime compartidos y backends de HAL. Genera archivos de salida en `EDU-CIAA/rover/out/`. La compilación no carga la placa.

Carga del rover, con la placa y herramientas disponibles:

```sh
make -C EDU-CIAA download
```

Diagnóstico independiente de I2C:

```sh
make -C EDU-CIAA PROGRAM_PATH=test PROGRAM_NAME=i2c_scan all
make -C EDU-CIAA PROGRAM_PATH=test PROGRAM_NAME=i2c_scan download
```

Este programa reemplaza temporalmente al rover y repite el escaneo cada 3 s. Consultar su [README](../EDU-CIAA/test/i2c_scan/README.md) para interpretar direcciones y estados. Recargar el rover al finalizar. La detección de dirección por sí sola no verifica el modelo del sensor.

## Arranque y protocolo V0

Mantener el sensor inmóvil hasta el mensaje de calibración completa. Con el firmware completo, una IMU que no inicializa impide iniciar las tareas normales; ToF ausente permite publicar orientación con distancia inválida.

La conexión serie utiliza 115200 baudios. Cada muestra contiene exactamente dos campos, en este orden, y termina en salto de línea:

```text
0.1234,0.4500
```

El primer campo es `theta_rad`; el segundo, `distancia_m`. La distancia inválida es `-1`. Los logs empiezan con `#` y deben ignorarse al interpretar muestras. No hay encabezado CSV como dato, timestamp, pose XY ni paquete de barrido. `imu_only` publica `theta,-1`; `tof_only` publica `0,distancia`.

## Processing

Abrir `Processing/flecha/flecha.pde` para orientación o `Processing/flecha_y_mapa_v1/flecha_y_mapa_v1.pde` para nube polar. Requieren Processing y su biblioteca Serial. Ajustar `portName` de `COM3` al puerto real y cerrar otros monitores que lo estén usando.

El nombre `flecha_y_mapa_v1` no indica una grilla V1: este sketch consume V0 y dibuja puntos con origen fijo. Para observar obstáculos, usar firmware completo con ambos sensores; los diagnósticos tienen un campo artificial.

## Verificación práctica propuesta

1. Registrar placa, sensores, versiones de herramientas y revisión compilada.
2. Comprobar arranque y calibración con robot quieto.
3. Girar un ángulo conocido y comprobar signo, escala y deriva de orientación.
4. Medir objetos a distancias conocidas dentro y fuera del filtro; verificar `-1` cuando corresponda.
5. Reiniciar sin ToF y comprobar que orientación sigue disponible; reiniciar sin IMU y comprobar el error de arranque.
6. Repetir en EDU-CIAA y registrar diferencias de tiempos, validez y enlace serie.

Son procedimientos de verificación pendientes, no resultados obtenidos en esta revisión. No se fijan tolerancias numéricas que el equipo todavía no acordó.
