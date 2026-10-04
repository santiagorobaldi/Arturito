# Estado de desarrollo

Revisión documental: 3 de octubre de 2026. Base inspeccionada: `a3a36acb065916da6bb2ed1b161c4a19d69ca79e` (`refactoring del worksapce`). El árbol de trabajo estaba limpio al comenzar. Este relevamiento es estático: no se realizaron builds, cargas ni ensayos físicos.

## Implementación y evidencia

| Componente | Evidencia en el árbol | Validación y límite |
| --- | --- | --- |
| HAL ESP32/EDU-CIAA | Cuatro backends por placa: IMU, ToF, tiempo, telemetría | Existencia y selección de fuentes inspeccionadas. |
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

El último commit concentra el runtime en `shared/layer3_app`, reduce los arranques a adaptadores de plataforma y divide las HAL combinadas en IMU, ToF, tiempo y telemetría. El agrupador `hal_sensors.h` se conserva. Se incorporaron diagnósticos aislados, documentos en `docs`, rutas compartidas en builds y configuración de editor. Se retiró `Processing/prueba_imu` y se alinearon los sketches restantes a metros y dos columnas. Las capas antiguas `layer3_robotics`/`layer4_app` ya no forman parte del árbol actual.

`EDU-CIAA/libs` contiene soporte de plataforma y bibliotecas, no funcionalidades del rover listas para usar. Se inspeccionaron inventario, selección de módulos y código relevante de integración; no se auditó exhaustivamente cada archivo de terceros. Los scripts de prueba/build heredados no demuestran por sí mismos cobertura del rover.

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
