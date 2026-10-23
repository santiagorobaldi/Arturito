#include "hal_telemetry_network.h"

#include <Arduino.h>
#include <WiFi.h>

#include <string.h>

namespace {
// Identidad de la red que crea el ESP32. No depende del router de casa.
constexpr char kAccessPointSsid[] = "Arturito";
constexpr char kAccessPointPassword[] = "arturito123";

// Processing se conecta a la IP del AP (192.168.4.1) en este puerto TCP.
constexpr uint16_t kTcpPort = 3333U;

// Limite para no aceptar tramas demasiado grandes por error.
constexpr size_t kMaxLineBytes = 127U;

// El ESP32 escucha conexiones y guarda el cliente actualmente atendido.
WiFiServer server(kTcpPort);
WiFiClient client;

// Evita volver a iniciar el AP si start() se llama mas de una vez.
bool started = false;
}

extern "C" bool hal_telemetry_network_start(void) {
    // Una segunda llamada no reinicia la red que ya esta funcionando.
    if (started) {
        return true;
    }

    // WIFI_AP convierte al ESP32 en el punto de acceso.
    WiFi.mode(WIFI_AP);

    // Crea la red protegida por la clave definida arriba.
    if (!WiFi.softAP(kAccessPointSsid, kAccessPointPassword)) {
        Serial.println("[WiFi AP] no se pudo iniciar el punto de acceso.");
        return false;
    }

    // Abre el servidor TCP y muestra los datos para conectar Processing.
    server.begin();
    started = true;
    Serial.printf("[WiFi AP] SSID: %s, IP: %s, TCP: %u\n",
                  kAccessPointSsid, WiFi.softAPIP().toString().c_str(), kTcpPort);
    return true;
}

extern "C" void hal_telemetry_network_poll(void) {
    // No hay nada que atender si el AP no arranco.
    if (!started) {
        return;
    }

    // Descarta el cliente anterior cuando Processing cierra la conexion.
    if (client && !client.connected()) {
        client.stop();
    }

    // Si estamos libres, toma una conexion TCP entrante.
    if (!client) {
        WiFiClient incoming = server.available();
        if (incoming) {
            client = incoming;
            // Evita que TCP espere para agrupar estas lineas cortas.
            client.setNoDelay(true);
            Serial.println("[WiFi AP] cliente TCP conectado.");
        }
    }
}

extern "C" bool hal_telemetry_network_send_line(const char *line) {
    // No se puede enviar si falta el texto o aun no hay cliente conectado.
    if (line == nullptr || !client || !client.connected()) {
        return false;
    }

    // El protocolo es una sola linea ASCII, terminada en LF, de hasta 127 bytes.
    const size_t length = strlen(line);
    if (length == 0U || length > kMaxLineBytes || line[length - 1U] != '\n') {
        return false;
    }

    // Rechaza saltos de linea dentro de la trama: cada llamada envia una fila.
    for (size_t i = 0U; i + 1U < length; ++i) {
        if (line[i] == '\r' || line[i] == '\n') {
            return false;
        }
    }

    // Envia exactamente los bytes de la fila, incluido el LF final.
    const size_t written = client.write(reinterpret_cast<const uint8_t *>(line), length);
    if (written != length) {
        // Descarta la conexion si salio solo una parte de la fila.
        client.stop();
        return false;
    }
    return true;
}
