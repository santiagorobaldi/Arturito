# Prueba de multiplexación I2C1

Primer paso del ensayo I2C0–I2C1 en EDU-CIAA. Configura P2_3 como I2C1_SDA y P2_4 como I2C1_SCL, inicializa ambos periféricos y configura sus relojes a 100 kHz. No configura un esclavo ni inicia transferencias.

## Compilar y cargar

Desde la carpeta `Arturito/firmware/EDU-CIAA`, en una terminal con las herramientas CIAA disponibles:

```text
make BOARD=edu_ciaa_nxp PROGRAM_PATH=test PROGRAM_NAME=i2c_pinmux all
make BOARD=edu_ciaa_nxp PROGRAM_PATH=test PROGRAM_NAME=i2c_pinmux download
```

No requiere cambiar program.mk ni el Makefile. La carga reemplaza el programa que tenga actualmente la placa.

## Verificación

Para este primer paso no hace falta puentear los buses ni conectar sensores. Abrir el puerto serie del USB DEBUG a 115200 baud, 8 bits, sin paridad, un bit de parada y sin control de flujo. El programa informa cada dos segundos FUNC1 para ambos pines y los relojes configurados, cercanos a 100000 Hz según la división del reloj. El LED verde indica que la lectura de los registros de multiplexación coincide; el azul alterna para indicar que el programa sigue ejecutándose.

La lectura de registros confirma la selección de función, pero no prueba el estado eléctrico del conector, las resistencias de pull-up ni la comunicación. Sin pull-ups en I2C1, no se debe interpretar la tensión de sus pines como verificación del bus. El siguiente paso será preparar un esclavo, conectar SDA con SDA y SCL con SCL con la placa apagada y comprobar escritura y lectura.

## Organización

- src/main.c: configuración y observación de la prueba.
- config.mk: selección de bibliotecas para este programa, sin FreeRTOS.
- out/: resultados generados al compilar.

No inicializar UART_232 después de configurar I2C1: comparte P2_3/P2_4. UART_USB utiliza otros pines.
