# Requisitos y verificación

Esta matriz deriva de `ARTURITO MAIN.tex`, sección 3, contrastada con el informe inicial y las decisiones actuales. Los identificadores se asignan en esta versión para futura trazabilidad; no eran IDs oficiales del informe. Los ensayos son propuestas y todavía no se ejecutaron como suite de aceptación. “Parcial” significa implementación incompleta o sin evidencia suficiente, no aprobación del requisito.

## Requisitos funcionales

| ID | Origen en MAIN | Requisito | Situación | Verificación propuesta |
| --- | --- | --- | --- | --- |
| RF-01 | Sensado/distancia HW | Barrido mecánico de sensor de 360° y ángulo propio | En discrepancia con ToF fijo V0 | Visar desvío y demostrar cobertura angular del método aprobado. |
| RF-02 | Sensado/distancia SW | Asociar distancia y ángulo al instante de muestreo | Parcial: pareja CSV sin timestamp | Registrar tiempo de adquisición y medir desfasaje con giro conocido. |
| RF-03 | Sensado/distancia SW | Rechazar fuera de rango y lecturas erróneas | Parcial: filtro 0,05–1,50 m y algunos errores HAL | Ensayar límites, fallo de bus y estados inválidos en ambas placas. |
| RF-04 | Sensado/distancia SW | Distancia y orientación en SI | Presente en interfaces | Comparar con distancia/ángulo de referencia; fijar tolerancias. |
| RF-05 | Sensado/distancia SW | Agrupar barrido, enviarlo a mapeo y estampar tiempo | Pendiente | Verificar contenido y delimitación de un barrido completo. |
| RF-06 | Sensado/odometría HW | Medir aceleración lineal en tres ejes | Pendiente en aplicación; V0 solo gyro Z | Ensayar lectura y unidades de los tres ejes si se mantiene alcance. |
| RF-07 | Sensado/odometría HW | Medir giro individual de cada rueda | Pendiente; AS5600 definitivo frente a pulsos del informe | Medir vueltas y continuidad angular por rueda en ambos sentidos. |
| RF-08 | Sensado/odometría SW | Convertir unidades, agregar timestamp y entregar lecturas a cinemática | Pendiente para ruedas; gyro en SI sin paquete temporal | Inspeccionar paquete y correspondencia temporal con referencia. |
| RF-09 | Comunicaciones HW | Enlace inalámbrico estándar | Pendiente; UART V0 | Ensayar transmisión y recepción por tecnología seleccionada. |
| RF-10 | Comunicaciones SW | Recibir pose/mapa y procesar START/STOP externos | Pendiente | Verificar transporte de mapa/pose y cambios de estado por comando. |
| RF-11 | Tracción HW | Dos motores DC, rueda loca y etapa de potencia | Requisito; DRV8833 seleccionado, sin control | Inspección mecánica y prueba independiente de cada motor. |
| RF-12 | Cinemática SW | Integrar pose diferencial desde sensores y entregarla a mapeo | Pendiente | Comparar recorridos rectos y giros con referencias externas. |
| RF-13 | Cinemática SW | Convertir consignas a velocidades por rueda y actuar en lazo abierto con PWM/sentido | Pendiente | Comprobar consignas, sentido, PWM y comportamiento físico. |
| RF-14 | Mapeo SW | Inicializar grilla con dimensiones/resolución predefinidas | Pendiente | Verificar límites, inicialización y representación de celdas. |
| RF-15 | Mapeo SW | Transformar medidas polares al mundo usando pose | Pendiente; gráfico V0 solo origen fijo | Casos geométricos con poses y distancias conocidas. |
| RF-16 | Mapeo SW | Trazar rayos y actualizar probabilidades libre/ocupado | Pendiente | Verificar celdas atravesadas, impacto y mediciones repetidas. |
| RF-17 | Mapeo SW | Entregar grilla y pose a navegación/comunicaciones | Pendiente | Comprobar consistencia de mapa/pose y consumidores. |
| RF-18 | Navegación SW | Evaluar zonas, elegir destino y emitir consignas sin colisionar | Pendiente | Escenarios de interior controlados con obstáculos. |

La visualización de una grilla es además un objetivo primario de la sección 2.2; se verificará contra celdas y pose conocidas cuando exista RF-14–17. El gráfico polar actual no cumple ese objetivo. SLAM, PID, teleoperación y rutas remotas quedan como objetivos secundarios, sin convertirlos en requisitos cumplidos ni fijarles APIs prematuras.

## Requisitos no funcionales

| ID | Origen | Condición | Datos faltantes / situación | Verificación propuesta |
| --- | --- | --- | --- | --- |
| RNF-01 | Rendimiento | Tasas mínimas de mapa y comunicación | `[FreqMinMapeo]`, `[FreqMinComunicaciones]` | Medir tasa y latencia con carga representativa. |
| RNF-02 | Rendimiento | Muestreo a intervalo fijo | `[T_s]`; 30 ms es espera nominal, no período garantizado | Medir intervalos y jitter; acordar tolerancia. |
| RNF-03 | Calidad de mapa | Resolución y dimensiones mínimas/máximas | `[ResolMinMapaOcup]`, `[DimMinMapaOcup]`, `[DimMaxMapaOcup]` | Verificar dimensiones, resolución y memoria consumida. |
| RNF-04 | Percepción | Rango de detección y precisión | `[DistMinObj]`, `[DistMaxObj]`, Rmin/Rmax y precisión | Ensayo con referencias; filtro de software no reemplaza calibración. |
| RNF-05 | Seguridad | Detener marcha por pérdida de radio | `[MaxSegSinConex]`; pendiente | Cortar enlace y medir tiempo hasta detención. |
| RNF-06 | Robustez | Reconexión automática | Pendiente | Interrumpir/restablecer enlace y verificar recuperación. |
| RNF-07 | Energía | Autonomía mínima | `[TiempoMinOperacion]`, batería y carga | Ensayo de operación con consumo registrado. |
| RNF-08 | Movimiento | Velocidad regulable | `[VelocidadMin]`, `[VelocidadMax]` | Medir desplazamiento/tiempo a distintas consignas. |
| RNF-09 | Entorno | Piso plano interior y altura mínima de obstáculos | `[AlturaMinObstaculos]` | Escenarios físicos con alturas/montajes documentados. |

## Contratos de implementación a verificar

| ID | Acuerdo actual | Verificación propuesta |
| --- | --- | --- |
| RC-01 | Tres capas y backends dentro de HAL | Revisar dependencias y compilación de ambos destinos. |
| RC-02 | FreeRTOS transversal y runtime común | Confirmar arranque, tareas y manejo de recursos en ambas placas. |
| RC-03 | ToF ausente no bloquea orientación | Reiniciar sin ToF y verificar CSV con `-1`. |
| RC-04 | Serie V0 a 115200, dos campos SI, logs `#` | Capturar salida y comprobar parser/valores en Processing. |

Antes de cerrar un requisito se debe registrar revisión de código, plataforma, montaje, criterio numérico, procedimiento, resultado y evidencia. No se asignan tolerancias ni fechas de cierre ausentes en las fuentes. Los conflictos de alcance deben resolverse mediante visado y, cuando corresponda, un ADR que reemplace el acuerdo anterior.
