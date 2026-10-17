# Arturito

Arturito es un proyecto desarrollado para la materia "Taller de Proyecto I" de la Facultad de Ingeniería de la Universidad Nacional de La Plata (UNLP).  

El proyecto propone un rover de tracción diferencial que combine mediciones de distancia, información inercial y giro de las ruedas para estimar su posición y construir un mapa de ocupación 2D del entorno el cual pueda visualizarse en un dispositivo externo, asi como tambien que sea capaz de navegar el entorno de manera autonoma para la generacion completa del mapa. 


## Organización del repositorio

| Carpeta | Contenido |
| --- | --- |
| `firmware/shared/config/` | Configuración general. |
| `firmware/shared/layer1_hal/` | Contratos de capa HAL. |
| `firmware/shared/layer2_drivers/` | Drivers para la utilización de sensores y modulos integrados. |
| `firmware/shared/layer3_app/` | Lógica compartida de la aplicación y coordinación de las tareas de lectura, procesamiento y envío de datos con FreeRTOS (Runtime). |
| `firmware/EDU-CIAA/` | Configuración de compilación, bibliotecas, inicialización de la aplicación y acceso al hardware de EDU-CIAA. |
| `firmware/ESP32/` | Proyecto PlatformIO con Arduino, inicialización de la aplicación y acceso al hardware de ESP32. |
| `docs/` | Documentación técnica y decisiones. |
| `KiCad/` | Esquemáticos y diseño de PCB. |
| `utils/` | Herramientas de PC |
| `utils/Processing/` | Visualización de orientación y lecturas de distancia. |


Arturito.code-workspace define las carpetas que VS Code abre como raíces del área de trabajo. Cada raíz puede tener su propia configuración del editor y ser reconocida por las extensiones. Por ejemplo, incluir firmware/ESP32 como raíz permite que PlatformIO encuentre su archivo platformio.ini y muestre las herramientas para compilar y cargar el firmware en la placa. Estas referencias no mueven ni duplican las carpetas del proyecto.


## Documentación

Este README ofrece una vista general del proyecto y sirve como punto de entrada a la documentación. Cada documento desarrolla un aspecto específico:

- [Descripción del sistema](docs/system-overview.md): explica cómo está compuesto el robot mediante el diagrama de bloques funcionales y describe la función de cada bloque y sus relaciones.
- [Arquitectura del firmware](docs/software-architecture.md): detalla las capas, sus interfaces y cómo se distribuyen en las carpetas del repositorio. 
- [Hardware](docs/hardware.md): reúne los componentes y sus características relevantes, su distribución en los bloques del sistema y sus conexiones. Sirve de guía para consultar los esquemáticos del proyecto en `KiCad/`.
- [Requisitos](docs/requirements.md): detalla qué debe cumplir el proyecto y cómo verificar cada requisito, vinculándolo con criterios de aceptación y pruebas de validación.
- [Preparación, compilación y ejecución](docs/build-and-run.md): indica qué herramientas se necesitan, cómo configurar el entorno, compilar y cargar el firmware en las placas, y poner en marcha la visualización y los programas de prueba.
- [Decisiones de diseño — ADR](docs/decisions/README.md): registra las decisiones importantes del proyecto, su contexto, justificación y consecuencias. Los demás documentos remiten aquí para explicar las elecciones de diseño.
- [Registro de desarrollo](docs/development-status.md): guarda el estado actual de los diferentes modulos o bloques a desarrollar para llevar un registro de lo realizado, lo validado, y lo que queda pendiente de desarrollo.

## Cómo empezar

Leé primero la [descripción del sistema](docs/system-overview.md) y luego la [arquitectura de software](docs/software-architecture.md). Para preparar las herramientas, compilar y ejecutar los programas de prueba, seguí [build-and-run.md](docs/build-and-run.md).

## Autores

Joaquín Guzmán, Tomás Gamarra, Santiago Robaldi y Federico Goncalves.
