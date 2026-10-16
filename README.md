# Arturito

Arturito es un proyecto desarrollado para la materia "Taller de Proyecto I" de la Facultad de Ingeniería de la Universidad Nacional de La Plata (UNLP). Surge de la idea de desarrollar un robot capaz de explorar un entorno desconocido y decidir cómo desplazarse a partir de lo que percibe, sin depender de un recorrido marcado en el piso ni de la intervención continua de un operador.

El proyecto propone un rover de tracción diferencial que combine mediciones de distancia, información inercial y giro de las ruedas para estimar su posición y construir un mapa de ocupación 2D del entorno el cual pueda visualizarse en un dispositivo externo, asi como tambien que sea capaz de navegar el entorno de manera autonoma para la generacion completa del mapa. 

## Hardware utilizado


| Componente | Cantidad | Función en Arturito |
| --- | ---: | --- | --- |
| Placa EDU-CIAA NXP | 1 | Plataforma principal de procesamiento y control del rover | 
| Placa NodeMCU V3 con módulo Wi-Fi| 1 | Comunicación inalámbrica con el dispositivo externo | 
| GY-521 MPU6050 | 1 | Sensado inercial del robot | 
| GY-53 VL53L0X | 1 | Medición de distancia al entorno mediante tiempo de vuelo (ToF) |
| AS5600 | 2 | Medición de posición angular de cada rueda para odometría | 
| DRV8833 | 1 | Etapa de potencia para controlar ambos motores DC | 
| Motorreductor GA12-N20 | 2 | Generar la tracción diferencial del rover |
| Celda Li-ion 18650 | 3 | Alimentación del sistema | 
| MT3608 DC-DC Step-Up | 1 | Elevar y regular la tensión del ramal de lógica |

Esta lista describe la selección del proyecto; el avance de integración está en [development-status.md](docs/development-status.md).


## Arquitectura de software

El firmware se organiza en tres capas: **Application → Device Drivers → HAL**. La aplicación reúne la lógica del robot; los drivers ofrecen el tratamiento de sensores y actuadores; y la HAL concentra el acceso al hardware mediante implementaciones específicas para cada plataforma. Esta separación permite reutilizar la lógica compartida y probar los módulos de manera independiente.

El kernel del sistema operativo de tiempo real FreeRTOS proporciona la ejecución concurrente y la sincronización como infraestructura transversal. La EDU-CIAA con el microcontrolador LPC4337 es la plataforma objetivo, mientras que otras plataformas como firmware/ESP32/Arduino se utiliza para validar sensores y firmware compartido.

El documento [software-architecture.md](docs/software-architecture.md) desarrolla más a fondo las responsabilidades de cada capa y su interacción.


## Organización del repositorio

| Carpeta | Contenido |
| --- | --- |
| `firmware/shared/config/` | Configuración general. |
| `firmware/shared/layer1_hal/` | Contratos de capa HAL. |
| `firmware/shared/layer2_drivers/` | Drivers para la utilización de sensores y modulos integrados. |
| `firmware/shared/layer3_app/` | Lógica compartida de la aplicación y coordinación de las tareas de lectura, procesamiento y envío de datos con FreeRTOS (Runtime). |
| `firmware/EDU-CIAA/` | Configuración de compilación, bibliotecas, inicialización de la aplicación y acceso al hardware de EDU-CIAA. |
| `firmware/ESP32/` | Proyecto PlatformIO con Arduino, inicialización de la aplicación y acceso al hardware de ESP32. |
| `utils/Processing/` | Visualización de orientación y lecturas de distancia. |
| `docs/` | Documentación técnica y decisiones. |
| `KiCad/` | Esquemáticos y diseño de PCB. |
| `utils/` | Herramientas de PC; actualmente Processing. |

`Arturito.code-workspace` abre únicamente la raíz del repositorio, mostrando docs, firmware, KiCad y utils sin entradas duplicadas. Para utilizar la configuración específica del editor de una plataforma, abrir directamente firmware/ESP32 o firmware/EDU-CIAA en otra ventana.

## Cómo empezar

Leé primero la [descripción del sistema](docs/system-overview.md) y luego la [arquitectura de software](docs/software-architecture.md). Para preparar las herramientas, compilar y ejecutar los programas de prueba, seguí [build-and-run.md](docs/build-and-run.md).

## Documentación

Este README ofrece una vista general del proyecto y sirve como punto de entrada a la documentación. Cada documento desarrolla un aspecto específico:

- [Descripción del sistema](docs/system-overview.md): explica cómo está compuesto el robot mediante el diagrama de bloques funcionales y describe la función de cada bloque y sus relaciones.
- [Arquitectura del firmware](docs/software-architecture.md): detalla las capas, sus interfaces y cómo se distribuyen en las carpetas del repositorio. 
- [Hardware](docs/hardware.md): reúne los componentes y sus características relevantes, su distribución en los bloques del sistema y sus conexiones. Sirve de guía para consultar los esquemáticos del proyecto en `KiCad/` y las decisiones de diseño asociadas.
- [Requisitos](docs/requirements.md): detalla qué debe cumplir el proyecto y cómo verificar cada requisito, vinculándolo con criterios de aceptación y pruebas de validación.
- [Preparación, compilación y ejecución](docs/build-and-run.md): indica qué herramientas se necesitan, cómo configurar el entorno, compilar y cargar el firmware en las placas, y poner en marcha la visualización y los programas de prueba.
- [Decisiones de diseño — ADR](docs/decisions/README.md): registra las decisiones importantes del proyecto, su contexto, justificación y consecuencias. Los demás documentos remiten aquí para explicar las elecciones de diseño.
- [Registro de desarrollo](docs/development-status.md): conserva el relevamiento de implementación y validación y las diferencias detectadas respecto de los informes. Su continuidad como documento de seguimiento está pendiente de definir.

## Autores

Joaquín Guzmán, Tomás Gamarra, Santiago Robaldi y Federico Goncalves.
