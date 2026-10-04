# ADR-0003: FreeRTOS como infraestructura transversal

Estado: aceptada. Registro documental: 3 de octubre de 2026.

## Contexto

El firmware combina adquisición, procesamiento, actuación y comunicación, con dependencias y necesidades temporales diferentes. Ambas plataformas deben sostener una coordinación común de la aplicación, aunque su integración con el entorno de ejecución sea específica.

## Decisión

Utilizar FreeRTOS como infraestructura transversal de planificación, comunicación y sincronización. Compartir la coordinación de adquisición, tratamiento de dispositivos y publicación de información, conservando la preparación y el inicio de ejecución propios de cada plataforma.

La separación en tareas pertenece al modelo de ejecución. Las responsabilidades funcionales permanecen en Application, Device Drivers y HAL; FreeRTOS no constituye una cuarta capa ni un servicio exclusivo de HAL.

## Justificación

**Coordinación concurrente.** Las tareas permiten organizar actividades con prioridades y esperas explícitas. Esto facilita separar la adquisición y el procesamiento de las operaciones de comunicación, que pueden tener tiempos de servicio diferentes.

**Infraestructura transversal.** La planificación y la sincronización afectan la coordinación de distintos módulos. Ubicar conceptualmente FreeRTOS dentro de HAL confundiría el acceso físico a dispositivos con el modelo de ejecución de toda la aplicación.

**Coordinación compartida.** Mantener una organización común permite conservar el flujo de adquisición, tratamiento y publicación al validar el firmware en otra placa. Las diferencias de preparación de plataforma y de integración con el scheduler se resuelven en su adaptación correspondiente.

## Enfoques de los antecedentes

El diseño académico contrasta el uso de tareas con un bucle secuencial que reúne las actividades del firmware. Ese bucle concentra la coordinación temporal en una única secuencia; la decisión adoptada permite expresarla mediante tareas y mecanismos de sincronización.

El PDF de arquitectura ubicaba RTOS junto a HAL. El acuerdo transversal distingue responsabilidades funcionales y unidades de ejecución. La elección no constituye una garantía automática de cumplimiento temporal ni de ausencia de pérdida de datos.

## Consecuencias

- Prioridades, períodos y memoria deben dimensionarse según los requisitos y verificarse con carga representativa.
- El intercambio debe definir si conserva cada muestra, comunica eventos o entrega el último estado; la elección de mutex, colas o notificaciones responde a esa semántica.
- Proteger un recurso compartido no garantiza asociación temporal de mediciones ni procesamiento de cada lectura una sola vez.
- La adaptación de plataforma debe preservar la semántica de ejecución común, incluyendo las convenciones de tiempo y memoria de las interfaces utilizadas.
- Los detalles de tareas y los resultados de verificación se documentan fuera del ADR para que el registro conserve la decisión y sus consecuencias.

## Referencias

Diseño preliminar de firmware, informe de avance y acuerdo de infraestructura transversal. La [arquitectura de software](../software-architecture.md) describe responsabilidades y contratos; [Estado de desarrollo](../development-status.md) registra implementación y validación.
