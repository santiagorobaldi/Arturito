# VL53L0X: API oficial ST 1.0.4 para EDU-CIAA

`Api/core/` contiene los cinco fuentes y encabezados originales de ST.
`Api/platform/` contiene la adaptacion EDU-CIAA en C. Se conserva la estructura
`Api/core/{inc,src}` y `Api/platform/{inc,src}` del paquete ST, incluyendo solo
los fuentes necesarios para EDU-CIAA. No se incluyen ejemplos de PC, transporte
Windows, binarios ni el driver Linux. Se conservan las licencias en los archivos.

Activacion: `USE_VL53L0X=y` en `rover/config.mk`. `module.mk` incorpora el nucleo
y la plataforma sin incluir los transportes de Windows ni sus trazas.
La biblioteca usa LPCOpen y los tipos de la sAPI seleccionada por el firmware;
actualmente `libs/sapi/module.mk` selecciona 0.5.2. No se cambia esa seleccion.

## Integracion con runtime

```
app_runtime.c
  -> driver_vl53l0x.c
  -> hal_vl53l0x.h
  -> hal_vl53l0x_educiaa.c
  -> API ST
  -> vl53l0x_platform.c
  -> vl53l0x_i2c_platform.c
  -> LPCOpen / I2C0
```

El runtime inicializa primero la IMU, cuyo backend sAPI configura I2C0.
El adaptador VL53L0X recibe ese bus preparado y ejecuta DataInit, consulta de
informacion, StaticInit, calibracion de referencia, gestion de SPAD y modo
single-ranging. No reinicializa el bus compartido. Una prueba independiente
debe configurar I2C0 antes de llamar a `hal_vl53l0x_init`.

La HAL ofrece inicio, consulta de disponibilidad y recoleccion de cada medicion.
El driver compartido `vl53l0x_get_distance` conserva el contrato de estado del
driver de la rama `feature/encoders` y entrega la distancia en centimetros,
convirtiendola desde metros al recoger una muestra valida. La capa `distance`
gestiona el tiempo de espera, valida el rango y aplica la interpolacion; la
aplicacion no llama directamente a la HAL del sensor. En EDU-CIAA, una muestra
solo es valida si la API tiene exito y `RangeStatus == 0`. El backend ESP32 usa
Adafruit y ofrece el mismo contrato de adquisicion.
El diagnostico ESP32 pasa de `tof_only` a `vl53l0x_only`.

## Separacion original de ST

- `vl53l0x_platform.c/.h`: contrato usado por el nucleo, contexto `Dev`,
  traduccion de errores y UpdateByte. Se conserva como intermediario.
- `vl53l0x_i2c_platform.c/.h`: firmas originales de transporte y conversion de
  word/dword. Las transferencias reemplazan la DLL Windows por LPCOpen en I2C0.
  La direccion ST se convierte de 0x52 a 0x29 dentro de esta capa.
- `vl53l0x_types.h`: tipos estandar del compilador, sin redeclaraciones.
- `vl53l0x_platform_log.h`: encabezado original, con trazas desactivadas.

El nucleo ST se conserva sin modificaciones. No se fusionan las dos capas.
Se conservan las firmas auxiliares originales del transporte: inicializacion,
cierre, esperas, GPIO, alimentacion y temporizadores. La inicializacion configura
I2C0 solo si se llama explicitamente; el adaptador del rover utiliza el bus ya
preparado por la IMU. El cierre no apaga un bus compartido.

GPIO, alimentacion, temporizadores y esperas programables distintas de cero
devuelven fallo: no se ha seleccionado su implementacion EDU-CIAA y el nucleo
de ranging integrado no las usa. Esto no afecta a `PollingDelay`, que conserva
la pausa acordada de 250 iteraciones. Adaptar esos hooks requiere revisar su
uso y el hardware correspondiente; no se simula exito para operaciones ausentes.
Cada funcion pendiente esta marcada como `NO SOPORTADA POR AHORA EN EDU-CIAA`
y conserva dentro de su cuerpo el codigo original ST comentado como referencia.
Ese codigo no se compila ni ejecuta: incluye dependencias de Windows/DLL y,
en algunos casos, solo trazas o implementaciones incompletas del propio ST.

Las validaciones de parametros y los retornos tras error evitan transferir
datos invalidos o convertir bytes no recibidos. `ReadMulti` puede dejar datos
parciales en caso de fallo: comprobar siempre su resultado.

## Pendientes para hardware

- Verificar identificacion, calibracion y distancias con el sensor conectado.
- Medir duracion de adquisicion e impacto sobre las tareas de Arturito.
- Evaluar relacion temporal entre distancia y orientacion de la IMU: el runtime
  conserva su secuencia y no se agrega sincronizacion temporal en este cambio.
- `PollingDelay` conserva 250 iteraciones con `nop`, como Adafruit. Es una
  espera activa no calibrada que no suspende tareas FreeRTOS.
- El acceso I2C es bloqueante y sin timeout ante un periferico que no progresa.
- No hay mutex interno: la aplicacion debe serializar el dispositivo y el bus.

Se verifico compilacion y enlace del rover completo con el toolchain ARM local.
Esto no constituye una prueba de funcionamiento en placa.
