# ADR-0004: Selección de encoders y sensor de distancia

Estado: aceptada. Registro: 3 de octubre de 2026.

## Contexto

Los antecedentes describen encoders por pulsos y un sensor GY-53; el proyecto ya acordó componentes concretos para ruedas y distancia.

## Decisión

Seleccionar AS5600 como encoder absoluto definitivo por rueda y VL53L0X como sensor de distancia. Mantener esa selección en contratos y documentación.

## Consecuencias y asuntos abiertos

AS5600 requiere definir continuidad de ángulo, vueltas e integración odométrica; no tiene driver todavía. VL53L0X ya tiene backends, cuyo comportamiento debe verificarse en ambas placas. No se conserva GY-53 como alternativa vigente ni se presenta la selección como montaje terminado.

## Evidencia

Acuerdos explícitos, diseño preliminar actualizado, informe de avance y backends `hal_tof_*`. La discrepancia con pulsos y GY-53 queda registrada en el estado.
