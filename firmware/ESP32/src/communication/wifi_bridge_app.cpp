#include "main.h"
#include "wifi_telemetry.h"
#include "esp32_board_config.h"
#include <Arduino.h>

static HardwareSerial source_uart(2);
/* Hasta 255 caracteres de texto mas el terminador; incluye los diagnosticos. */
static char line[256];
static size_t length = 0U;
static bool dropping = false;
static bool ready = false;

void app_init(void)
{
    Serial.println("PUENTE UART -> WIFI");
    source_uart.begin(BOARD_TELEMETRY_UART_BAUD, SERIAL_8N1,
                      BOARD_TELEMETRY_UART_RX_GPIO, BOARD_TELEMETRY_UART_TX_GPIO);
    ready = wifi_telemetry_start();
}

void app_update(void)
{
    if (!ready) {
        return;
    }
    wifi_telemetry_poll();
    while (source_uart.available()) {
        const char byte = (char)source_uart.read();
        if (byte == '\n') {
            if (!dropping && length > 0U) {
                line[length] = '\0';
                wifi_telemetry_send_line(line);
            }
            length = 0U;
            dropping = false;
        } else if (byte != '\r' && !dropping) {
            if (length < sizeof(line) - 1U) {
                line[length++] = byte;
            } else {
                dropping = true;
            }
        }
    }
}
