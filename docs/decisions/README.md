# Decisiones de arquitectura

Un ADR registra una decisión, su contexto, justificación y consecuencias. Estos registros iniciales formalizan acuerdos existentes; no afirman que todo el hardware o software esté integrado.

| ADR | Decisión | Estado |
| --- | --- | --- |
| [0001](0001-firmware-architecture.md) | Tres capas y HAL de periféricos con backends de plataforma | Aceptada. |
| [0002](0002-target-platform.md) | EDU-CIAA objetivo y ESP32 para validación | Aceptada. |
| [0003](0003-freertos-runtime.md) | FreeRTOS transversal con coordinación compartida | Aceptada. |
| [0004](0004-sensor-selection.md) | AS5600 definitivo y VL53L0X para distancia | Aceptada. |

Fecha de registro documental: 3 de octubre de 2026. La fecha exacta de cada acuerdo previo no se infiere de esta fecha. Los detalles V0 del informe de avance se describen en los documentos técnicos y en las discrepancias, sin elevarlos a nuevas decisiones definitivas de alcance académico.

Para una decisión futura usar el siguiente número y registrar: estado (propuesta, aceptada, reemplazada), contexto, decisión, justificación, alternativas documentadas, consecuencias, referencias y asuntos abiertos. Si cambia una decisión aceptada, conservar el registro y enlazar el ADR que la reemplaza.

## Distribución de la documentación

Los documentos temáticos describen el sistema y sus contratos; los ADR concentran las razones de las elecciones y sus consecuencias. Cuando un documento necesita explicar por qué se eligió una solución, enlaza al ADR correspondiente en lugar de repetir su justificación. El estado de implementación y los resultados de pruebas se registran en el estado de desarrollo.

Las alternativas se incluyen cuando existe evidencia de ellas. No se atribuyen evaluaciones, comparaciones ni motivos de selección que el equipo no haya documentado; los fundamentos faltantes se señalan para revisión.
