# Arturito

Workspace del rover diferencial. El alcance vigente y las decisiones de ingenieria estan en [docs/informe_avance.tex](docs/informe_avance.tex).

## Estructura

- `docs/`: documentacion del proyecto. `ARTURITO MAIN.tex` se conserva como entrega congelada; el informe de avance es la referencia viva.
- `shared/config/`: constantes comunes de compilacion y sensores.
- `shared/layer1_hal/`: contratos C de la HAL, independientes de la placa.
- `shared/layer2_drivers/`: drivers compartidos, sin dependencias de Arduino o SAPI.
- `shared/layer3_app/`: runtime FreeRTOS compartido para adquirir, actualizar drivers y publicar; pose/mapa siguen pendientes.
- `esp32/src/layer1_hal/`: implementaciones ESP32 de tiempo, IMU y ToF.
- `esp32/src/tests/`: programas de diagnostico aislado para IMU y ToF.
- `processing/`: visualizacion polar V0 por serie.
- `edu-ciaa/`: HAL y adaptador de arranque EDU-CIAA. Usa el runtime y drivers compartidos; compila, falta validar sensores en la placa.

ESP32 y EDU-CIAA comparten las tres tareas FreeRTOS. Arduino ya inicia su scheduler antes de `setup()`; `main()` de CIAA crea el bootstrap común y luego inicia el scheduler. La inicializacion/calibracion ocurre dentro del bootstrap, con el scheduler activo.

## ESP32

Desde `esp32/`, el entorno predeterminado `esp32dev` es el firmware completo:

```sh
pio run
pio run -t upload
```

Los perfiles de diagnostico se seleccionan de forma explicita:

```sh
pio run -e imu_only
pio run -e tof_only
pio run -e imu_only -t upload
pio run -e tof_only -t upload
```

Para volver a cargar el firmware completo, usar `pio run -t upload` sin `-e`.

El firmware EDU-CIAA se compila desde la raiz con `make -C edu-ciaa`. Este comando solo compila; no carga la placa.

## Protocolo V0

A 115200 baud: lineas de log prefijadas con `#` y muestras CSV `theta_rad,distancia_m`. Processing consume ese contrato.

Los sketches usan `COM3` por defecto. En otra PC, cambiar `portName` al puerto serie asignado al ESP32.
