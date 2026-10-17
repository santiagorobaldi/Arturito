# Estado de desarrollo

Actualización: 17 de octubre de 2026.

Este documento registra el avance de los módulos del firmware, organizado por las capas definidas en la [arquitectura de software](software-architecture.md). Permite identificar qué está pendiente, qué tiene implementación, qué fue probado individualmente y qué ya funciona dentro del sistema final. Refleja el estado actual; no es un historial de cambios.

## Cómo leer y completar las tablas

| Columna | Qué describe y cómo se completa |
| --- | --- |
| Módulo | Nombre del módulo.|
| Responsabilidad | Función que debe resolver el módulo, expresada brevemente. |
| Plataforma | ESP32, EDU-CIAA o Compartido. Separar las filas cuando cambien la implementación, el estado o la evidencia entre plataformas. Compartido identifica código común, no acredita pruebas en ambas placas. |
| Estado | Uno de los cuatro estados definidos a continuación. Se asigna según la evidencia disponible. |
| Notas | Decisiones pendientes, limitaciones, condiciones de la prueba y comentarios importantes. Al validar, registrar plataforma, resultado y fecha si se conoce. |
| Test independiente | Enlace al archivo que permite probar individualmente el módulo. 

### Estados

| Estado | Significado |
| --- | --- |
| **A definir** | Se identificó la necesidad del módulo, pero todavía se están planificando su responsabilidad o interfaz y no existe implementación. |
| **Implementado** | Existe una implementación, pero todavía no hay evidencia suficiente de funcionamiento mediante una prueba. Compilar y enlazar correctamente no basta para avanzar de estado. |
| **Validado individualmente** | Se ejecutó una prueba del módulo y cumplió el comportamiento esperado en las condiciones registradas. |
| **Integrado correctamente** | El módulo forma parte del sistema final y cumplió las pruebas de integración definidas junto con los otros módulos. |


La columna Test independiente identifica una prueba centrada en el módulo; puede utilizar sus dependencias habituales. Un test que combina varios módulos se registra en la sección de pruebas de integración, sin presentarlo como prueba independiente de cada componente.

## Capa HAL

Las interfaces comunes están en `firmware/shared/layer1_hal`; cada plataforma proporciona su implementación. Los estados siguientes corresponden a los adaptadores propios del proyecto, no a la madurez de las bibliotecas externas que utilizan.

| Módulo | Responsabilidad | Plataforma | Estado | Notas | Test independiente |
| --- | --- | --- | --- | --- | --- |
| [MPU6050](../firmware/ESP32/src/layer1_hal/hal_mpu6050_esp32.cpp) | Inicializar y adquirir mediciones mediante la biblioteca de plataforma. | ESP32 | Implementado | Utiliza Adafruit. El test IMU disponible ejercita también el driver y el cálculo del ángulo; falta registrar una prueba específica del adaptador. | Sin test independiente |
| [MPU6050](../firmware/EDU-CIAA/rover/src/layer1_hal/hal_mpu6050_educiaa.c) | Inicializar y adquirir mediciones mediante la biblioteca de plataforma. | EDU-CIAA | Implementado | Utiliza sAPI sin modificarla; conserva su calibración interna. Falta validar lecturas y detección de errores en placa. | Sin test independiente |
| [VL53L0X](../firmware/ESP32/src/layer1_hal/hal_vl53l0x_esp32.cpp) | Inicializar y obtener distancias mediante la biblioteca de plataforma. | ESP32 | Implementado | Utiliza Adafruit. La prueba disponible de distancia también incluye el driver compartido. | Sin test independiente |
| [VL53L0X](../firmware/EDU-CIAA/rover/src/layer1_hal/hal_vl53l0x_educiaa.c) | Inicializar y obtener distancias mediante la biblioteca de plataforma. | EDU-CIAA | Implementado | Utiliza la API ST 1.0.4 y su adaptación I²C. Falta validar físicamente inicialización, distancias y tiempos. | Sin test independiente |
| [Tiempo](../firmware/ESP32/src/layer1_hal/hal_time_esp32.cpp) | Proporcionar tiempo en milisegundos y esperas. | ESP32 | Implementado | Utilizado por las pruebas; sin registro de comprobación individual de tiempos. | Sin test independiente |
| [Tiempo](../firmware/EDU-CIAA/rover/src/layer1_hal/hal_time_educiaa.c) | Proporcionar tiempo en milisegundos y esperas. | EDU-CIAA | Implementado | Utilizado por la prueba V0; sin registro de comprobación individual de tiempos. | Sin test independiente |
| [Telemetría serie](../firmware/ESP32/src/layer1_hal/hal_telemetry_esp32.cpp) | Enviar líneas de texto por el puerto serie. | ESP32 | Implementado | La prueba V0 utiliza este adaptador. Falta registrar validación de la implementación actual. | Sin test independiente |
| [Telemetría serie](../firmware/EDU-CIAA/rover/src/layer1_hal/hal_telemetry_educiaa.c) | Enviar líneas de texto por el puerto serie. | EDU-CIAA | Implementado | La prueba V0 utiliza este adaptador. Falta registrar validación de la implementación actual. | Sin test independiente |
| Acceso a AS5600 | Proporcionar el acceso de plataforma requerido por los encoders. | EDU-CIAA | A definir | El equipo confirmó la usabilidad del segundo bus I²C. La interfaz y la implementación para los encoders siguen pendientes. | Sin test independiente |
| Acceso a DRV8833 | Proporcionar el acceso a las señales de control de los motores. | EDU-CIAA | A definir | Pendiente definir interfaz y configuración de GPIO/PWM. | Sin test independiente |
| Comunicación EDU-CIAA–NodeMCU | Proporcionar acceso a la UART por hardware entre placas. | EDU-CIAA / NodeMCU | A definir | Enlace previsto para transmitir información hacia Wi-Fi; falta definir e implementar su interfaz de software. NodeMCU es la placa de comunicación, distinta de la plataforma ESP32 utilizada para pruebas. | Sin test independiente |


## Capa Device Drivers

| Módulo | Responsabilidad | Plataforma | Estado | Notas | Test independiente |
| --- | --- | --- | --- | --- | --- |
| [MPU6050](../firmware/shared/layer2_drivers/driver_mpu6050.c) | Inicializar el sensor, entregar aceleración y velocidad angular en los tres ejes y calibrar el offset del giroscopio. | Compartido | Implementado | Compila en ambas plataformas. Falta registrar validación individual de mediciones y calibración con los adaptadores actuales. En EDU-CIAA corrige el error residual de sAPI. | Sin test independiente |
| [VL53L0X](../firmware/shared/layer2_drivers/driver_vl53l0x.c) | Inicializar el sensor, obtener la distancia y comprobar sus límites de validez. | Compartido | Implementado | Existe una prueba dedicada al sensor en ESP32. Falta registrar su resultado con la versión actual. Los límites de distancia todavía dependen de `config.h`. | [vl53l0x_only.cpp](../firmware/ESP32/src/tests/vl53l0x_only.cpp) — ESP32 |
| AS5600 | Obtener la posición angular de cada encoder. | Compartido | A definir | Pendientes interfaz, implementación y manejo de los dos dispositivos. | Sin test independiente |
| DRV8833 | Ofrecer control de los dos canales del puente H. | Compartido | A definir | Pendientes interfaz e implementación. El uso físico del módulo informado por el equipo no valida un driver de firmware todavía inexistente. | Sin test independiente |

## Capa Application

Los nombres de los módulos pendientes describen funciones previstas; su división definitiva y sus interfaces siguen por acordar.

| Módulo | Responsabilidad | Plataforma | Estado | Notas | Test independiente |
| --- | --- | --- | --- | --- | --- |
| Odometría / estimación de pose | Estimar orientación y posición del robot a partir de sus mediciones internas. | Compartido | A definir | Por decisión del equipo se retiró el módulo independiente de odometría. El cálculo angular existente pertenece a los tests `perception_v0` e `imu_only`; no constituye el módulo definitivo de pose. | Sin test independiente |
| Control de movimiento | Traducir las consignas de movimiento en acciones sobre los motores utilizando realimentación. | Compartido | A definir | Pendientes estrategia, interfaces y criterios de prueba. | Sin test independiente |
| Mapeo de ocupación | Incorporar mediciones de distancia y pose a una representación de ocupación del entorno. | Compartido | A definir | La visualización polar del prototipo V0 no implementa una grilla de ocupación. | Sin test independiente |
| Navegación | Decidir los movimientos del robot según su objetivo y la información disponible. | Compartido | A definir | Pendientes comportamiento e interfaces. | Sin test independiente |
| Comunicación y comandos | Definir e intercambiar la información del sistema con el dispositivo externo mediante el NodeMCU. | EDU-CIAA / NodeMCU | A definir | Pendientes protocolo, mensajes y comportamiento ante fallos. El CSV serie de V0 pertenece al ensayo. | Sin test independiente |
| Coordinación del sistema final | Organizar inicialización, tareas, acceso a módulos y sincronización. | EDU-CIAA | A definir | El antiguo `app_runtime` se convirtió en `perception_v0`; sus tareas corresponden al test y no se consideran el runtime final. | Sin test independiente |

## Tests disponibles

Esta sección identifica los programas de prueba actuales y su alcance. Su existencia no modifica automáticamente el estado de los módulos anteriores.

| Test | Plataforma | Alcance | Evidencia disponible / pendiente |
| --- | --- | --- | --- |
| [imu_only.cpp](../firmware/ESP32/src/tests/imu_only.cpp) | ESP32 | Ensayo de MPU6050, calibración y estimación angular local al test; entrega ángulo por serie. | Compilación satisfactoria. Combina adquisición y cálculo angular; falta registrar ejecución física de la versión actual. |
| [vl53l0x_only.cpp](../firmware/ESP32/src/tests/vl53l0x_only.cpp) | ESP32 | Prueba dedicada de inicialización y lectura de distancia mediante el driver y su adaptador. | Compilación satisfactoria; falta registrar ejecución física de la versión actual. |
| [i2c_scan](../firmware/EDU-CIAA/test/i2c_scan/src/main.c) | EDU-CIAA | Diagnóstico de dispositivos y comunicación I²C. | Compilación registrada; identificar condiciones y resultado físico antes de usarlo como evidencia de validación. |
| [i2c_pinmux](../firmware/EDU-CIAA/test/i2c_pinmux/src/main.c) | EDU-CIAA | Diagnóstico de selección de pines para I²C. | El equipo confirmó la usabilidad de los pines del segundo bus. Falta asociar esa comprobación a un test y registrar su resultado concreto. |
| [i2c_loopback](../firmware/EDU-CIAA/test/i2c_loopback/src/main.c) | EDU-CIAA | Comunicación entre I2C0 maestro e I2C1 esclavo con puentes entre buses. | Compilación registrada. El programa permite comprobar transferencias; no acredita por sí solo lectura de encoders ni I2C1 como maestro. |

### Prueba de integración del prototipo V0

| Test | Plataforma | Alcance | Evidencia disponible / pendiente |
| --- | --- | --- | --- |
| [perception_v0.c](../firmware/shared/tests/perception_v0/perception_v0.c) | ESP32 / EDU-CIAA | Adquisición MPU6050 y VL53L0X, cálculo del ángulo y envío CSV `theta_rad,distancia_m` mediante tareas FreeRTOS. | Compilación y enlace satisfactorios del test en ambas plataformas; también del alias ESP32 `esp32dev`. Falta probar la versión actual en placa y verificar la visualización con Processing. |

Las entradas de plataforma son [perception_v0.cpp para ESP32](../firmware/ESP32/src/tests/perception_v0.cpp) y [main.c para EDU-CIAA](../firmware/EDU-CIAA/test/perception_v0/src/main.c). El cálculo angular está dentro del archivo común del test; `imu_only` conserva su propia copia para ser independiente. Ninguno depende de un módulo `odometry.c` de Application.

El ensayo V0 utiliza la última muestra disponible y mutex para compartir datos. Quedan por comprobar asociación temporal IMU/ToF, pérdidas de muestras y período efectivo. La nube polar de Processing supone un origen fijo; no acredita pose XY, navegación ni integración del sistema final.

