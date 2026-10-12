# ADR-0001: Tres capas y abstracción del acceso a dispositivos

Estado: aceptada. Registro documental inicial: 3 de octubre de 2026.
Aclaración del alcance: 7 de octubre de 2026.

## Contexto

El firmware debe separar la lógica del rover, los drivers de sus
dispositivos y el acceso dependiente de la arquitectura. Algunos
dispositivos requieren implementar su protocolo; otros cuentan con
bibliotecas existentes ligadas a una plataforma.

## Decisión

Organizar el firmware como **Application → Device Drivers → HAL**.
Application contiene estimación de orientación y pose, cinemática,
mapeo, navegación y coordinación del sistema. Device Drivers implementa
el funcionamiento de sensores y actuadores, incluyendo su configuración,
interpretación de lecturas y tratamiento propio de la variable física.
HAL abstrae los buses y servicios dependientes de la plataforma.

Cuando un driver implementa el protocolo del dispositivo, utiliza
operaciones de HAL como lectura y escritura por I2C. Cuando existe
una biblioteca de dispositivo dependiente de plataforma, HAL puede
encapsularla mediante un adaptador de periférico que utiliza el driver.
La HAL de periféricos es una modalidad de adaptación para esos casos,
no una sustitución general de los drivers por tratamiento de la lógica
del rover.

## Justificación

**Separación de responsabilidades.** El driver conoce el sensor o
actuador; HAL conoce el mecanismo de acceso de la plataforma; Application
conoce el robot. Así se distinguen los registros del dispositivo de
los registros del microcontrolador y se mantiene la lógica del rover
fuera de ambas capas inferiores.

**Portabilidad.** Las interfaces de HAL permiten conservar el protocolo
de un driver al cambiar de arquitectura. Los adaptadores permiten
mantener también esa independencia cuando se utiliza una biblioteca
ligada a una placa o framework.

**Reutilización de bibliotecas.** Encapsular una implementación existente
evita reescribir su acceso al chip. El adaptador normaliza la interfaz
para que esa biblioteca no imponga dependencias de plataforma al driver
compartido o a la aplicación.

**Tres capas.** Los algoritmos de robótica y la coordinación del sistema
se organizan como módulos de Application. Sus responsabilidades pueden
separarse sin añadir otra capa jerárquica.

## Consecuencias

- Los drivers pueden conocer los registros del dispositivo, pero el
  acceso específico del microcontrolador queda encapsulado en HAL.
- La conversión de lecturas y el suavizado de ruido propios del sensor
  pueden pertenecer al driver; estimar orientación o pose del rover
  pertenece a Application.
- Los adaptadores de bibliotecas deben conservar el significado, las
  unidades y la validez de la interfaz del dispositivo.
- Los servicios de buses y los adaptadores de periféricos pueden
  coexistir dentro de HAL, según las necesidades de cada dispositivo.

## Referencias y aclaración

La [arquitectura de software](../software-architecture.md) describe estas
responsabilidades. El diseño preliminar presenta servicios de buses y
el informe de avance describe el uso de una HAL de periféricos. La
aclaración del equipo establece cómo conviven ambos enfoques.

Esta aclaración corrige la redacción inicial de este ADR, que presentaba
la HAL de periféricos como frontera general y asignaba integración de
orientación a los drivers. Esa interpretación no representa el diseño
definido por el equipo. Las diferencias del código y de los informes
previos deben registrarse en [Estado de desarrollo](../development-status.md).
