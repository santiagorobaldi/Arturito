# Arturito

Arturito es un proyecto desarrollado para la materia "Taller de Proyecto I" de la Facultad de Ingeniería de la Universidad Nacional de La Plata (UNLP). Surge de la idea de desarrollar un robot capaz de explorar un entorno desconocido y decidir cómo desplazarse a partir de lo que percibe, sin depender de un recorrido marcado en el piso ni de la intervención continua de un operador.

El proyecto propone un rover de tracción diferencial que combine mediciones de distancia, información inercial y giro de las ruedas para estimar su posición y construir un mapa de ocupación 2D del entorno el cual pueda visualizarse en un dispositivo externo, asi como tambien que sea capaz de navegar el entorno de manera autonoma para la generacion completa del mapa. 

## Arquitectura de software

El firmware se organiza en tres capas: **Application → Device Drivers → HAL**. La aplicación reúne la lógica del robot; los drivers ofrecen el tratamiento de sensores y actuadores; y la HAL concentra el acceso al hardware mediante implementaciones específicas para cada plataforma. Esta separación permite reutilizar la lógica compartida y probar los módulos de manera independiente.

El kernel del sistema operativo de tiempo real FreeRTOS proporciona la ejecución concurrente y la sincronización como infraestructura transversal. La EDU-CIAA con el microcontrolador LPC4337 es la plataforma objetivo, mientras que ESP32 se utiliza para validar sensores y firmware compartido.

El documento [software-architecture.md](docs/software-architecture.md) desarrolla las responsabilidades de cada capa, la justificación de esta organización y los contratos de la implementación actual.

## Hardware principal

- MPU6050 para medición inercial.
- VL53L0X para distancia por tiempo de vuelo.
- AS5600 como encoder absoluto de cada rueda.
- DRV8833 para la etapa de potencia de los motores.

Esta lista describe la selección del proyecto; el avance de integración está en [development-status.md](docs/development-status.md).

## Organización del repositorio

| Carpeta | Contenido |
| --- | --- |
| `shared/config/` | Configuración general. |
| `shared/layer1_hal/` | Contratos de HAL. |
| `shared/layer2_drivers/` | Drivers para la utilización de sensores. |
| `shared/layer3_app/` | Lógica compartida de la aplicación y coordinación de las tareas de lectura, procesamiento y envío de datos con FreeRTOS (Runtime). |
| `EDU-CIAA/` | Configuración de compilación, bibliotecas, inicialización de la aplicación y acceso al hardware de EDU-CIAA. |
| `ESP32/` | Proyecto PlatformIO con Arduino, inicialización de la aplicación y acceso al hardware de ESP32. |
| `Processing/` | Visualización de orientación y lecturas de distancia. |
| `docs/` | Documentación técnica, decisiones e informes académicos. |

`Arturito.code-workspace` reúne estas carpetas para trabajar en el proyecto. En sistemas sensibles a mayúsculas deben respetarse los nombres reales indicados arriba.

## Cómo empezar

Leé primero la [descripción del sistema](docs/system-overview.md) y luego la [arquitectura de software](docs/software-architecture.md). Para preparar las herramientas, compilar y ejecutar los programas de prueba, seguí [build-and-run.md](docs/build-and-run.md).

## Documentación

- [Sistema](docs/system-overview.md): bloques funcionales y flujo de información.
- [Software](docs/software-architecture.md): responsabilidades, interfaces y concurrencia.
- [Hardware](docs/hardware.md): componentes, conexiones conocidas y datos por completar.
- [Requisitos](docs/requirements.md): trazabilidad y criterios de verificación.
- [Build y ejecución](docs/build-and-run.md): procedimientos y protocolo serie.
- [Estado de desarrollo](docs/development-status.md): implementación, antecedentes de validación y discrepancias.
- [Decisiones de arquitectura](docs/decisions/README.md): acuerdos y sus consecuencias.


## Autores

Joaquín Guzmán, Tomás Gamarra, Santiago Robaldi y Federico Goncalves.
