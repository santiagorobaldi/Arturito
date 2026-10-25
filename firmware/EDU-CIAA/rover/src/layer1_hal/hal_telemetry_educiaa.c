#include "hal_telemetry.h"
#include "educiaa_board_config.h"
#include "sapi.h"

bool hal_telemetry_init(void)
{
    uartConfig(BOARD_TELEMETRY_UART, BOARD_TELEMETRY_UART_BAUD);
    return true;
}

void hal_telemetry_update(void)
{
    /* El envio por UART no necesita tareas periodicas. */
}

bool hal_telemetry_send_line(const char *line)
{
    if (line == NULL) {
        return false;
    }
    uartWriteString(BOARD_TELEMETRY_UART, line);
    uartWriteString(BOARD_TELEMETRY_UART, "\n");
    return true;
}

void hal_telemetry_write(const char *line)
{
    uartWriteString(UART_USB, line);
    uartWriteString(UART_USB, "\r\n");
}
