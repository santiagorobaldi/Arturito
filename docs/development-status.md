# Estado de desarrollo

Revisión documental: 3 de octubre de 2026. Base inspeccionada: `a3a36acb065916da6bb2ed1b161c4a19d69ca79e` (`refactoring del worksapce`). El árbol de trabajo estaba limpio al comenzar. Este relevamiento es estático: no se realizaron builds, cargas ni ensayos físicos.

## Implementación y evidencia

| Componente | Evidencia en el árbol | Validación y límite |
| --- | --- | --- |
| HAL firmware/ESP32/EDU-CIAA | Cuatro backends por placa: IMU, ToF, tiempo, telemetría | Existencia y selección de fuentes inspeccionadas. |
| Drivers compartidos | MPU con offset/zona muerta/integración y filtro de distancia | Algoritmos presentes; precisión sin ensayo en esta revisión. |
| Aplicación | Bootstrap, tres tareas, dos mutex y CSV | Runtime compartido presente; falta evidencia temporal por muestra. |
| Build ESP32 | `esp32dev`, `imu_only`, `tof_only` | Informe de avance declara builds exitosos y carga; no reproducidos aquí. |
| Build EDU-CIAA | `rover/config.mk`, FreeRTOS y fuentes compartidas | Informe de avance declara enlace exitoso; no reproducido aquí. |
| Visualización | Flecha y nube polar con contrato V0 | Informe de avance declara validación por Santiago. No constituye grilla. |
| Diagnóstico I2C | Programa independiente y README | Permite investigar direcciones, NACK e identidad de IMU. |
| Motores/encoders | Selección AS5600/DRV8833 en documentos | Sin drivers ni control implementados. |
| Pose XY, grilla, navegación | No se encontraron módulos | Pendientes. |
| Radio/comandos | Sin implementación en la aplicación inspeccionada | UART actual; Wi-Fi/Bluetooth y START/STOP pendientes. |

## Cambios recientes de organización

El último commit concentra el runtime en `firmware/shared/layer3_app`, reduce los arranques a adaptadores de plataforma y divide las HAL combinadas en IMU, ToF, tiempo y telemetría. El agrupador `hal_sensors.h` se conserva. Se incorporaron diagnósticos aislados, documentos en `docs`, rutas compartidas en builds y configuración de editor. Se retiró `utils/Processing/prueba_imu` y se alinearon los sketches restantes a metros y dos columnas. Las capas antiguas `layer3_robotics`/`layer4_app` ya no forman parte del árbol actual.

`firmware/EDU-CIAA/libs` contiene soporte de plataforma y bibliotecas, no funcionalidades del rover listas para usar. Se inspeccionaron inventario, selección de módulos y código relevante de integración; no se auditó exhaustivamente cada archivo de terceros. Los scripts de prueba/build heredados no demuestran por sí mismos cobertura del rover.

## Fuentes y mantenimiento

La precedencia de esta documentación es: decisiones explícitas del proyecto, código/build actual, documentación del repositorio e informes académicos anteriores. Una decisión describe el diseño aprobado; el código describe lo implementado. Cuando difieren, se registra la diferencia sin dar por implementada la decisión.

- `ARTURITO MAIN.tex`: informe académico y requisitos de referencia; se conserva.
- `Diseño preliminar de firmware.tex`: antecedente de tres capas con HAL de buses.
- `informe_avance.tex`: registro del equipo, decisiones y resultados declarados; se reutilizó su contenido técnico.
- `explicando_firmware.tex`: guía extensa del runtime, HAL y builds; complemento técnico.
- `CONTEXTO_PROYECTO_ROVER.tex`: fotografía anterior con referencias a carpetas/estados que ya cambiaron; el informe de avance lo declara absorbido.

Los Markdown organizan la documentación por tema y los ADR registran acuerdos. Los `.tex` se mantienen sin reescritura. El informe de avance se declara autoridad viva y pide actualizarse con cambios; esta propuesta documental todavía requiere visado del equipo para acordar cómo evitar duplicación de autoridad entre ese archivo y los nuevos documentos.

Los PDF académicos consultados fuera del repositorio son antecedentes: el informe inicial del 30/09/2026 y el diseño de arquitectura del 01/10/2026. No se agregaron copias ni enlaces a rutas externas al repositorio.

## Contradicciones y dudas abiertas

| Tema | Evidencia divergente | Tratamiento en esta versión |
| --- | --- | --- |
| Capas | PDF de arquitectura: cuatro capas y RTOS dentro de HAL; acuerdo y árbol: tres capas | Documentar tres capas y RTOS transversal; conservar antecedente. |
| Frontera HAL/drivers | Diseño preliminar: HAL de buses; código y avance: HAL de periféricos | Describir contratos reales y señalar cambio de responsabilidad. |
| Plataforma | EDU-CIAA objetivo; avance: desarrollar ESP32 primero; workspace llama CIAA “Port futuro” | Distinguir destino del proyecto y orden de validación. El port ya tiene código. |
| Scan | MAIN exige giro mecánico del sensor 360°; avance usa ToF fijo y giro del chasis | Registrar desvío V0; aceptación del requisito académico pendiente. |
| Distancia | PDF de arquitectura menciona GY-53; selección y código usan VL53L0X | VL53L0X vigente. |
| Encoder | MAIN exige pulsos/giros; AS5600 es definitivo | Medición absoluta seleccionada; falta especificar conversión de vueltas y tratamiento de discontinuidad. |
| IMU | MAIN pide aceleración 3 ejes; aplicación usa gyro Z | Requisito completo pendiente. Diagnóstico contempla ID `0x72`; confirmar módulo real. |
| Signo de theta | Avance menciona convención de signo; getter devuelve theta sin inversión | Verificar ejes y signo físico; no inferir una convención de montaje. |
| Sincronía | MAIN exige asociación temporal y timestamps; runtime comparte últimas muestras sin timestamp | CSV no demuestra simultaneidad ni garantiza una lectura por actualización. |
| Período | 30 ms configurados; tareas esperan después del trabajo | Valor nominal; no afirmar tasa garantizada de 33,3 Hz. |
| ToF CIAA | Backend inicia single-shot y lee rango tras esperar que se limpie START | Revisar si espera suficiente para resultado listo y cómo interpreta estado/calidad; no equivale al chequeo `RangeStatus` de ESP32. |
| Errores I2C | Diagnóstico advierte booleanos sAPI poco fiables; HAL usa esas operaciones | Validar fallas de bus y lecturas; gyro cero puede confundir error con reposo. |
| Estado de CIAA | Mensaje de commit dice que funciona en ambas; avance aún pide ensayo físico | Resultado físico no confirmado por esta revisión; solicitar registro de placa/sensores. |
| Nombres | Carpetas versionadas en mayúsculas; workspace/textos en minúsculas | Usar nombres reales en documentación; compatibilidad sensible a mayúsculas pendiente. |
| Radio y mapa | Objetivos académicos primarios; implementación UART/nube V0 | Mantener requisitos; no dar por cumplidos radio ni grilla. |
| Parámetros | MAIN contiene valores entre corchetes sin definición | Mantener pendientes; no sustituirlos silenciosamente por constantes del firmware. |

## Etapas de integración

**V0: percepción con origen fijo.** El ToF permanece fijo al chasis y el ángulo se estima integrando el giroscopio Z. La visualización supone `x = y = 0`: representa puntos polares mientras el chasis gira. No demuestra odometría de traslación ni navegación autónoma, y no implica que el firmware controle ese giro. La distancia y el ángulo se publican por serie.

**V1: pose y grilla.** Con los AS5600 integrados se podrá estimar el desplazamiento de las ruedas, incorporar la geometría del chasis y transformar distancias al marco del mundo. La grilla requerirá trazado de rayos y actualización de ocupación. Es una etapa prevista, todavía sin implementación en el código inspeccionado.

Generar una grilla desde una pose odométrica no basta para afirmar que existe SLAM: el objetivo secundario exige un método que estime conjuntamente mapa y localización.

## Próximos pasos para el equipo

Visar contratos y jerarquía documental; confirmar modelo/montaje de sensores y evidencia física de EDU-CIAA; definir criterios numéricos de aceptación; resolver timestamps y sincronización antes de mapear en movimiento; integrar AS5600 y DRV8833 con contratos medibles; desarrollar pose y grilla. El informe de avance propone radio como próximo avance funcional, pero esa prioridad no se redefine aquí.

## Aclaración de responsabilidades del 7 de octubre de 2026

El equipo precisa que Device Drivers conserva el funcionamiento propio
del dispositivo y HAL abstrae el acceso de plataforma. Los adaptadores
de periféricos permiten reutilizar bibliotecas existentes sin sustituir
los servicios de buses como modelo general. La estimación de orientación
del rover pertenece a Application. El informe de avance y el relevamiento
anterior asignan integración de orientación al driver de MPU6050; esa
asignación difiere de esta definición estructural. No se modificó código
ni se dio por realizada una reubicación de funciones. Véase
[ADR-0001](decisions/0001-firmware-architecture.md).

## Separación MPU6050 y orientación — 12 de octubre de 2026

Se separó el tratamiento de mediciones del cálculo de orientación:

- `firmware/shared/layer2_drivers/driver_mpu6050.h/.c`: inicialización, adquisición de aceleración/giro X/Y/Z y calibración de offsets residuales. Muestra pública única, parámetros directos y resultado booleano.
- `firmware/shared/layer1_hal/hal_mpu6050.h`: contrato específico de adquisición MPU6050. Reemplaza hal_imu.h y reutiliza el tipo de muestra del driver.
- `firmware/ESP32/src/layer1_hal/hal_mpu6050_esp32.cpp`: adaptador Adafruit MPU6050 2.2.9. Mantiene los rangos ESP32 previos de ±2 g y ±250 grados/s; biblioteca declarada para esp32dev e imu_only.
- `firmware/EDU-CIAA/rover/src/layer1_hal/hal_mpu6050_educiaa.c`: adaptador de sAPI existente, sin cambios a la biblioteca. sAPI configura ±16 g y ±2000 grados/s y calibra automáticamente al inicializar. El driver compartido calibra el error residual de sus lecturas, por decisión explícita del usuario.
- `firmware/shared/layer3_app/odometry.h/.c`: orientación relativa de un eje, zona muerta e integración trapezoidal usando el instante de cada muestra. La primera muestra establece referencia; reset modifica orientación y restart_tracking conserva el ángulo y descarta el historial temporal.
- `app_runtime.c`: adquiere mediante el driver; adjunta tiempo, validez y secuencia. Procesa una secuencia una sola vez. Ante lectura fallida o salto de secuencia reinicia seguimiento temporal conservando el ángulo. Se conserva el intercambio de última muestra mediante mutex; sigue sin ser una cola de todas las mediciones.
- `firmware/ESP32/src/tests/imu_only.cpp`: utiliza driver y odometry. Conserva el formato theta,-1. El programa completo conserva theta,distancia y los mensajes de calibración.

La espera inicial de 2 s, las 1000 muestras separadas por 2 ms y la zona muerta de 1 grado/s permanecen en el código que coordina la aplicación, no como dependencias de configuración del driver. La calibración ahora cubre los tres ejes de giro. Application conserva el montaje actual: giro Z del sensor usado directamente como giro Z del robot; otro montaje exige una transformación explícita.

### Límites de la implementación

El éxito booleano no garantiza detectar todos los fallos I2C. Adafruit getEvent 2.2.9 devuelve true sin propagar el resultado de la lectura interna. sAPI 0.5.2 utiliza una comprobación !resultado sobre una función que devuelve 1/-1. Ambas bibliotecas se mantuvieron intactas. El driver comprueba valores finitos y conserva la salida ante fallos detectados, pero un valor obsoleto finito puede pasar esa comprobación. Ensayos de desconexión y soporte de lectura con errores fiables quedan pendientes.

Los módulos tienen estado único y requieren acceso serializado. El arranque calibra antes de iniciar adquisición. Después, una tarea accede a MPU6050 y otra accede a odometry; publicación utiliza la copia protegida del resultado. El tiempo adjunto se toma al terminar la lectura, no del instante físico interno de conversión del sensor. No se conocen movimientos durante intervalos perdidos.

La integración corresponde a diseño de tres capas, pero el driver de distancia y sus accesos directos desde Application no se refactorizaron en este cambio. No se afirma conformidad completa del resto del firmware.

### Verificación

Compilaron correctamente los tres entornos ESP32: esp32dev, imu_only y tof_only. No se cargó hardware ni se verificaron precisión, tiempos o calibración físicamente.

El primer build EDU-CIAA encontró una cabecera C++ que el toolchain no resolvía (bits/c++config.h) al compilar hal_tof_educiaa.cpp. Los módulos driver_mpu6050.c, odometry.c, app_runtime.c y el nuevo adaptador MPU6050 compilaron. Se comprobó después el build completo agregando temporalmente CPLUS_INCLUDE_PATH al directorio arm-none-eabi de cabeceras C++ del paquete; el rover completo compiló y enlazó correctamente con esa preparación temporal. No se modificaron Makefile, ToF ni bibliotecas para resolver el entorno.

### Adaptadores EDU-CIAA en C — 12 de octubre de 2026

Por preferencia del equipo, los cuatro adaptadores del rover EDU-CIAA usan C: hal_mpu6050_educiaa.c, hal_tof_educiaa.c, hal_time_educiaa.c y hal_telemetry_educiaa.c. Las declaraciones privadas del ToF y el estado MPU6050 utilizan static; se retiraron namespace y static_cast. config.mk incorpora esos archivos mediante SRC. La biblioteca sAPI y las interfaces públicas permanecen sin cambios por esta conversión.

El rover compiló y enlazó correctamente utilizando el compilador C, sin CPLUS_INCLUDE_PATH adicional. El ajuste de cabeceras C++ registrado anteriormente deja de ser necesario para este programa. ESP32 conserva sus adaptadores C++ para utilizar Arduino y Adafruit. No se realizaron cargas ni pruebas físicas.
## Integracion VL53L0X — 14 de octubre de 2026

La API ST 1.0.4 esta integrada en `firmware/EDU-CIAA/libs/vl53l0x`, activada con `USE_VL53L0X=y`. El adaptador `hal_vl53l0x_educiaa.c` sustituye la implementacion manual de registros y utiliza la inicializacion/calibracion ST y `VL53L0X_PerformSingleRangingMeasurement`. La interfaz compartida es `hal_vl53l0x.h`; runtime conserva su secuencia de adquisicion y su politica de tareas.

Los archivos `hal_tof_*` se renombraron a `hal_vl53l0x_*` en ambas plataformas. El diagnostico ESP32 ahora se llama `vl53l0x_only`. Se comprobaron compilacion y enlace del rover EDU-CIAA y los entornos ESP32 `esp32dev` y `vl53l0x_only`. No se cargo hardware.

Pendientes: medir tiempos y asociacion ToF/IMU, validar inicializacion y distancias, resolver timeout I2C y revisar concurrencia si se agregan usuarios del bus. `PollingDelay` tiene 250 iteraciones activas con `nop`; no suspende la tarea FreeRTOS. La biblioteca y sus limites estan documentados en `firmware/EDU-CIAA/libs/vl53l0x/README.md`.

### Correccion de capas y estructura — 15 de octubre de 2026

El driver compartido se llama `driver_vl53l0x.c/.h`, antes `driver_distancia`.
La aplicacion inicializa y lee mediante ese driver; ya no llama a la HAL del
VL53L0X directamente. El driver delega inicializacion y acceso en
`hal_vl53l0x`, y valida los limites de distancia antes de entregar la lectura.
La tarea de procesamiento recibe ese valor ya validado.

La biblioteca conserva `firmware/EDU-CIAA/libs/vl53l0x/Api/core/{inc,src}` y
`Api/platform/{inc,src}`. Solo se incorporan los componentes de la API necesarios
para EDU-CIAA, sin ejemplos PC ni transporte Windows. La carpeta auxiliar
`VL53L0X_1.0.4/Api/platform/educiaa` y los logs generados por las verificaciones
se retiraron del workspace a un respaldo temporal. La descarga original deja
de contener esa copia adicional. La biblioteca integrada es la copia activa.

### Conservacion de las capas de ST — 15 de octubre de 2026

Por indicacion del equipo se conserva la separacion original: `platform.c`
traduce las operaciones ST y llama a `i2c_platform.c`, que implementa el
transporte EDU-CIAA mediante LPCOpen. Se restauro `vl53l0x_i2c_platform.h`
con sus firmas originales. El nucleo coincide byte a byte con la descarga ST.
La compilacion y el enlace del rover con ambas capas pasaron; no se probo
hardware. Las funciones auxiliares no utilizadas y aun sin backend devuelven
fallo explicito, segun lo documentado en la biblioteca.

## Organización del repositorio — 16 de octubre de 2026

La raíz contiene docs (documentación), firmware (EDU-CIAA, ESP32 y shared), KiCad (electrónica) y utils (Processing). La reorganización de fuentes ya estaba versionada al comenzar esta revisión. Las antiguas carpetas EDU-CIAA y ESP32 de la raíz contenían únicamente archivos ignorados: productos de construcción, configuración generada y dependencias descargadas. Se verificó su contenido antes de retirarlas; las dependencias PlatformIO se trasladaron a firmware/ESP32/.pio/libdeps.

Se actualizaron Arturito.code-workspace, la excepción de .gitignore para la configuración EDU-CIAA, las rutas documentales y las instrucciones de los diagnósticos. El workspace final abre solamente la raíz Arturito: se retiraron los accesos adicionales de plataforma porque duplicaban su aparición en el explorador. La configuración específica de cada plataforma puede utilizarse abriendo su carpeta directamente en otra ventana. La configuración del editor EDU-CIAA incorpora Application, FreeRTOS y la API ST; selecciona cabeceras de sAPI 0.5.2 en vez de buscar indiscriminadamente todas las versiones. PlatformIO regeneró su configuración local para la ubicación nueva.

Las rutas relativas hacia shared en config.mk y platformio.ini siguen siendo válidas porque las tres carpetas se movieron juntas. No se cambiaron fuentes, dependencias declaradas ni reglas del Makefile para esta reorganización.

Verificación desde las rutas nuevas: compilación y enlace satisfactorios de ESP32 esp32dev, imu_only y vl53l0x_only; rover EDU-CIAA; diagnósticos EDU-CIAA i2c_scan, i2c_pinmux e i2c_loopback. Se comprobaron las rutas del workspace, los directorios de cabeceras explícitos y los enlaces locales Markdown. No se cargó hardware ni se verificó funcionamiento físico. Se conservaron los cambios preexistentes de KiCad/DesignSchematic/.history.
