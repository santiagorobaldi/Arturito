
#include "hal_telemetry.h"

#include "sapi.h"

/* Envia una linea por la UART USB de EDU-CIAA. */
void hal_telemetry_write(const char *line)
{
    uartWriteString(UART_USB, line);
    uartWriteString(UART_USB, "\r\n");
}
