#include "sapi.h"
#include "chip.h"
#include <stdio.h>

static void configureI2cPins(void)
{
    const uint16_t mode = SCU_MODE_INACT | SCU_MODE_INBUFF_EN |
                          SCU_MODE_ZIF_DIS | SCU_MODE_FUNC1;

    /* I2C0 tiene pads dedicados; I2C1 usa pines multiplexados. */
    Chip_SCU_I2C0PinConfig(I2C0_STANDARD_FAST_MODE);
    Chip_SCU_PinMuxSet(2, 3, mode); /* P2_3: I2C1_SDA, antes U3_TXD. */
    Chip_SCU_PinMuxSet(2, 4, mode); /* P2_4: I2C1_SCL, antes U3_RXD. */
}

int main(void)
{
    boardInit();
    uartConfig(UART_USB, 115200);

    configureI2cPins();
    Chip_I2C_Init(I2C0);
    Chip_I2C_Init(I2C1);
    Chip_I2C_SetClockRate(I2C0, 100000);
    Chip_I2C_SetClockRate(I2C1, 100000);

    while (TRUE) {
        const unsigned int sdaFunction = LPC_SCU->SFSP[2][3] & 7u;
        const unsigned int sclFunction = LPC_SCU->SFSP[2][4] & 7u;
        const bool_t muxOk = sdaFunction == 1u && sclFunction == 1u;

        printf("P2_3 FUNC%u; P2_4 FUNC%u; multiplexacion %s\r\n",
               sdaFunction, sclFunction, muxOk ? "OK" : "ERROR");
        printf("Relojes configurados: I2C0=%lu Hz; I2C1=%lu Hz\r\n",
               (unsigned long) Chip_I2C_GetClockRate(I2C0),
               (unsigned long) Chip_I2C_GetClockRate(I2C1));
        printf("Sin transferencias: esta prueba no verifica ACK ni cableado.\r\n");

        gpioWrite(LEDG, muxOk);
        gpioToggle(LEDB);
        delay(2000);
    }
}
