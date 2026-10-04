# ADR-0001: Arquitectura de firmware en tres capas y HAL de periféricos

Estado: aceptada. Registro documental: 3 de octubre de 2026.

## Contexto

El rover combina tratamiento de sensores, estimación de posición, mapeo, navegación y actuación. La lógica debe poder reutilizarse entre plataformas sin depender de las bibliotecas de una placa. Los antecedentes describen una organización de cuatro capas y una HAL de buses; el acuerdo del proyecto establece tres capas con una HAL de periféricos.

## Decisión

Organizar el firmware como **Application → Device Drivers → HAL**. Cinemática, estimación, mapeo y navegación son módulos de Application. Los drivers compartidos realizan el tratamiento de dispositivos; HAL encapsula el acceso físico mediante backends específicos de plataforma.

HAL ofrece servicios de periféricos con magnitudes y semántica comunes. Sus backends pueden utilizar bibliotecas de sensores, bibliotecas de placa o acceso de menor nivel. La conversión de formatos y unidades de esas bibliotecas queda en HAL; la calibración, integración y validación del dispositivo quedan en los drivers compartidos.

## Justificación

**Separación de responsabilidades.** Los algoritmos del robot trabajan con pose, mapa y consignas, mientras los drivers tratan dispositivos y HAL resuelve su acceso físico. Esta frontera limita el impacto de cambiar una biblioteca o un mecanismo de comunicación sobre los algoritmos de aplicación.

**Tres niveles de abstracción.** Cinemática, estimación, mapeo y navegación comparten conceptos del dominio del robot. Separarlos en módulos dentro de Application permite delimitar sus responsabilidades sin agregar otro nivel jerárquico entre robótica y aplicación.

**Portabilidad y verificación.** Las interfaces comunes permiten conservar el tratamiento y los algoritmos al adaptar una placa. También permiten suministrar mediciones conocidas para verificar un módulo sin depender del sensor físico y comprobar un backend contra su contrato por separado.

**HAL de periféricos.** Esta frontera permite adaptar bibliotecas existentes de sensores al contrato del proyecto. Evita que los drivers compartidos deban conocer los registros del chip y las particularidades de cada biblioteca de acceso. Las bibliotecas pueden resolver inicialización, transacciones y lecturas; HAL normaliza el resultado que reciben las capas superiores.

## Enfoques de los antecedentes

La organización de cuatro capas separaba algoritmos de robótica y aplicación. El acuerdo de tres capas reúne esas funciones en Application, conservando su separación mediante módulos.

Una HAL de buses ofrecería operaciones genéricas, como leer bytes por I2C, y dejaría el protocolo del sensor en el driver compartido. Esa frontera favorece compartir el protocolo del chip, pero exige desarrollarlo o adaptarlo fuera de las bibliotecas específicas de plataforma. El proyecto adopta la frontera de periféricos descrita arriba. Estos enfoques aparecen en los antecedentes; no se presenta esta comparación como un ensayo experimental de alternativas.

## Consecuencias

- Los drivers compartidos quedan independientes de Arduino, sAPI y otras bibliotecas de plataforma.
- Los detalles de acceso a un chip pueden variar entre backends, que deben mantener unidades, signos, validez y errores equivalentes.
- Una biblioteca de acceso al chip puede llamarse driver, pero ocupa un papel distinto al driver compartido de Arturito.
- La coordinación puede acceder a servicios de HAL para adquirir datos o transportarlos, respetando el tratamiento y los límites de cada capa; la arquitectura no exige que toda llamada atraviese las tres capas.
- Cambiar a una HAL de buses o modificar el número de capas requiere un ADR que reemplace esta decisión.

## Referencias

[Descripción estructural](../software-architecture.md), diseño preliminar de firmware e informe de avance. Las diferencias con los antecedentes y el cumplimiento de esta estructura se registran en [Estado de desarrollo](../development-status.md).
