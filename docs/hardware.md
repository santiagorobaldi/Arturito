# Hardware

Este documento describe los principales componentes físicos utilizados
por Arturito y la función que cumplen dentro del sistema.

El estado actual de integración de cada componente se encuentra en
[Development Status](development-status.md).

## Componentes

| Elemento | Cantidad | Descripción
| --- | ---: | --- |
| EDU-CIAA NXP / LPC4337 | 1 | Plataforma principal de procesamiento y control 
| ESP32 | 1 | Plataforma para comunicación con el dispositivo externo
| MPU6050 | 1 | Medición inercial de 6 ejes (Acelerómetro + Giroscopio)
| Gy-53 VL53L0X | 1 | Medición de distancia mediante ToF para detección de obstáculos
| AS5600 | 2 | Encoder magnetico para medición de posición angular de las ruedas 
| DRV8833 | 1 | Puente H para control de potencia de los motores DC (Soporte para doble motor)
| Motores DC N20 con caja reductora| 2 | Tracción del rover 
| Ruedas motrices | 2 | Ruedas de goma para la tracción del robot
| Rueda loca | 1 | Rueda para apoyo y estabilidad del chasis en el movimiento
| Bateria 18650 recargable | 3 | Bateria Li-ion de 3.7V y 8800mAh para alimentacion de todo el sistema
| Cargador USB para Baterias 18650 | 1 | Cargador para las baterias
| Step-Up Mt3608 DC-DC | 1 | Eleva la tensión hasta 28V

Cualquier justificacion de por que elegimos estos componentes debe ir aqui con link a la ADR correspondiente, aunque todavia no se si decision de componentes entra en ADR.

## Organización del hardware

[diagrama general]

## Esquemático y conexiones

El esquema eléctrico y el pinout definitivo se incorporarán cuando se
complete la integración del hardware.

La documentación deberá incluir:

- distribución de alimentación;
- buses de comunicación;
- asignación de pines;
- conexión de sensores;
- conexión de encoders;
- conexión del driver y motores.

