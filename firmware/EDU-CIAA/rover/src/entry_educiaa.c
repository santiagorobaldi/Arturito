
#include "sapi.h"

#include "main.h"

/* Inicializa la plataforma y ejecuta la aplicación compartida. */
int main(void)
{
    /* Inicializa los recursos generales de EDU-CIAA. */
    boardConfig();

    /* Habilita la UART del puerto USB de depuración. */
    uartConfig(UART_USB, 115200);

    /* Inicializa la aplicación seleccionada. */
    app_init();

    /* Ejecuta continuamente la aplicación compartida. */
    while (1) {
        app_update();
    }

    return 0;
}
