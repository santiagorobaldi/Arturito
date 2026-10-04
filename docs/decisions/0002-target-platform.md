# ADR-0002: Plataforma objetivo y validación

Estado: aceptada. Registro documental: 3 de octubre de 2026.

## Contexto

El proyecto tiene EDU-CIAA como plataforma objetivo y dispone de ESP32 para prototipado y validación de sensores y firmware. Se necesita distinguir el destino del rover del orden de trabajo utilizado para verificar sus módulos.

## Decisión

Mantener EDU-CIAA/LPC4337 como plataforma objetivo y ESP32 como plataforma de validación. Compartir la lógica de aplicación y los drivers, con backends de HAL e integración de ejecución específicos para cada plataforma.

## Justificación

La disponibilidad de ESP32 permite realizar pruebas de sensores y lógica compartida sin esperar la integración completa en EDU-CIAA. Mantener los mismos contratos de dispositivos permite aprovechar esas pruebas durante el desarrollo para la plataforma objetivo.

Esta separación conserva el destino acordado y evita mantener dos aplicaciones independientes. La razón documentada para utilizar ESP32 es su función de validación; no se establece una superioridad de rendimiento o costo entre placas sin una evaluación que la respalde.

## Consecuencias

- Validar primero en ESP32 no cambia la plataforma objetivo.
- Cada plataforma requiere su backend, configuración de compilación y preparación del entorno de ejecución.
- Compartir fuentes y contratos no demuestra equivalencia física: tiempos, errores de acceso y comportamiento de sensores deben verificarse en ambas plataformas.
- Un ensayo satisfactorio en ESP32 no acredita por sí solo el funcionamiento en EDU-CIAA.

## Referencias

Acuerdo de plataforma e informe de avance. La [arquitectura de software](../software-architecture.md) describe la separación entre código compartido y backends; los resultados de integración corresponden a [Estado de desarrollo](../development-status.md).
