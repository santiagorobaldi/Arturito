# Comunicación entre I2C0 e I2C1

Ensayo en una EDU-CIAA: I2C0 es maestro a 100 kHz e I2C1 es esclavo con dirección de 7 bits 0x42. Sin sensores conectados. El maestro envía cuatro bytes y verifica que el esclavo recibió el contenido exacto; luego lee cuatro bytes que el esclavo devuelve invertidos bit a bit. Una secuencia variable permite comprobar que la respuesta corresponde a la iteración actual.

## Conexión

Apagar y desconectar la placa antes de colocar los puentes:

- I2C0_SDA con RS232_TXD (P2_3 del LPC4337, seleccionado como I2C1_SDA).
- I2C0_SCL con RS232_RXD (P2_4 del LPC4337, seleccionado como I2C1_SCL).

Confirmar las etiquetas y ubicación física en el esquemático de la propia placa. Usar cables cortos. No conectar adaptadores RS232, UART ni sensores a estas líneas durante el ensayo. No conectar las señales a 5 V. Ambos extremos pertenecen a la misma placa y comparten masa; las resistencias de pull-up de I2C0 a 3,3 V sirven a las líneas unidas. Una vez separados los buses, cada uno necesitará sus propias pull-up.

## Compilar y cargar

Desde `Arturito/firmware/EDU-CIAA`:

```text
make BOARD=edu_ciaa_nxp PROGRAM_PATH=test PROGRAM_NAME=i2c_loopback all
make BOARD=edu_ciaa_nxp PROGRAM_PATH=test PROGRAM_NAME=i2c_loopback download
```

La carga reemplaza el programa anterior. Abrir el puerto serie del USB DEBUG a 115200, 8N1, sin control de flujo.

## Resultado esperado

Cada dos segundos se informa escritura DONE, cuatro bytes recibidos, lectura DONE y resultado PASS. En la primera iteración TX es 12 34 00 A5 y RX debe ser ED CB FF 5A. El LED verde indica PASS, el rojo FAIL y el azul alterna. Dejar al menos 30 iteraciones y verificar que fallidas se mantenga en cero.

- NAK de dirección: el esclavo no reconoció la dirección; revisar puentes, pines y configuración.
- NAK de datos: se rechazó un byte; revisar la longitud y el estado del esclavo.
- DONE con FAIL: hubo una transferencia, pero los bytes o su cantidad no coinciden.
- TIMEOUT: el ensayo supera 100 ms; se reinician los periféricos y se detiene la prueba. Apagar, revisar SDA/SCL, pull-up y cableado, y volver a encender.

El límite temporal depende de SysTick funcionando. No implementa recuperación de un bus retenido por un dispositivo externo. La prueba no configura I2C1 como maestro: invertir los roles será una validación posterior antes de conectar los encoders en buses separados.

## Código

src/main.c contiene la configuración, el maestro atendido por polling y el esclavo atendido por I2C1_IRQHandler. No se imprime dentro de la interrupción. LPCOpen avanza los punteros y reduce los tamaños de los buffers; el callback del esclavo los rearma después de cada operación. RX reserva ocho bytes para recibir una trama de cuatro con margen para el manejo de ACK de esta versión de LPCOpen.

La API del maestro recibe 0x42; Chip_I2C_SlaveSetup de esta versión espera la dirección desplazada un bit (0x42 << 1). Ambas representan la misma dirección de 7 bits.

config.mk habilita LPCOpen y sAPI sin FreeRTOS. La prueba no modifica el firmware rover ni las bibliotecas.
