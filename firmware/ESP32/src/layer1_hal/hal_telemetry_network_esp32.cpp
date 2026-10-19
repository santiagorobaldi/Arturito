#include "hal_telemetry_network.h"

#include <Arduino.h>
#include <WiFi.h>
#include <lwip/sockets.h>

// La plantilla es opcional: el proyecto también compila antes de configurar
// la red, y en ese caso no intenta conectarse con valores de ejemplo.
#if __has_include("wifi_secrets.h")
#include "wifi_secrets.h"
#else
#define ARTURITO_WIFI_CONFIGURED 0
#define ARTURITO_WIFI_SSID ""
#define ARTURITO_WIFI_PASSWORD ""
#endif

#ifndef HAL_TELEMETRY_TCP_PORT
#define HAL_TELEMETRY_TCP_PORT 3333
#endif

namespace {
// Los intentos de conexión se espacian para no trabar el programa ni llenar
// el monitor serie si el router está apagado.
constexpr uint16_t kTcpPort = HAL_TELEMETRY_TCP_PORT;
constexpr uint32_t kReconnectIntervalMs = 5000U;
constexpr uint32_t kConnectAttemptTimeoutMs = 15000U;
constexpr uint32_t kServerStartRetryIntervalMs = 5000U;
constexpr size_t kMaxLineBytes = 127U;

// Este ensayo atiende un solo cliente a la vez.
WiFiServer server(kTcpPort, 1U);
WiFiClient client;

// Estado que poll() conserva entre llamadas.
bool started = false;
bool serverStarted = false;
bool serverStartAttempted = false;
bool connectPending = false;
uint32_t lastConnectAttemptMs = 0U;
uint32_t connectStartedMs = 0U;
uint32_t lastServerStartAttemptMs = 0U;

void closeClient() {
    // WiFiClient::stop() también es seguro si el cliente ya se fue.
    if (client) {
        client.stop();
    }
}

void stopServer() {
    if (serverStarted) {
        server.stop();
    }
    serverStarted = false;
    serverStartAttempted = false;
}

void startServerIfNeeded(uint32_t now) {
    // El servidor solo tiene sentido mientras haya conexión con el router.
    if (WiFi.status() != WL_CONNECTED || serverStarted) {
        return;
    }
    if (serverStartAttempted &&
        static_cast<uint32_t>(now - lastServerStartAttemptMs) < kServerStartRetryIntervalMs) {
        return;
    }

    server.begin();
    serverStartAttempted = true;
    lastServerStartAttemptMs = now;
    if (server) {
        serverStarted = true;
        Serial.printf("[WiFi TCP] servidor escuchando en puerto %u\n", kTcpPort);
    } else {
        // begin() no devuelve estado; operator bool indica si quedó escuchando.
        // Cerrar el socket fallido permite volver a intentarlo sin filtrar FDs.
        server.stop();
        Serial.println("[WiFi TCP] no se pudo iniciar el servidor; se reintentará.");
    }
}

void beginConnectAttempt(uint32_t now) {
    Serial.println("[WiFi TCP] intentando conectar al router...");
    WiFi.begin(ARTURITO_WIFI_SSID, ARTURITO_WIFI_PASSWORD);
    lastConnectAttemptMs = now;
    connectStartedMs = now;
    connectPending = true;
}
} // namespace

extern "C" bool hal_telemetry_network_start(void) {
#if !ARTURITO_WIFI_CONFIGURED
    // No intentar usar el SSID de ejemplo por accidente.
    Serial.println("[WiFi TCP] falta wifi_secrets.h configurado; no se inicia Wi-Fi.");
    return false;
#else
    if (ARTURITO_WIFI_SSID[0] == '\0' || ARTURITO_WIFI_PASSWORD[0] == '\0') {
        Serial.println("[WiFi TCP] SSID o contraseña vacíos; no se inicia Wi-Fi.");
        return false;
    }

    if (started) {
        return true;
    }

    // La reconexión la maneja poll(), así podemos espaciar los intentos.
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
    started = true;
    beginConnectAttempt(millis());
    return true;
#endif
}

extern "C" void hal_telemetry_network_poll(void) {
    if (!started) {
        return;
    }

    const uint32_t now = millis();
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED) {
        // Informar la IP una sola vez por cada conexión recuperada.
        if (connectPending) {
            connectPending = false;
            Serial.printf("[WiFi TCP] conectado; IP local: %s\n", WiFi.localIP().toString().c_str());
        }
        startServerIfNeeded(now);

        if (client && !client.connected()) {
            closeClient();
            Serial.println("[WiFi TCP] cliente desconectado; esperando otro cliente.");
        }
        if (!client) {
            // available() devuelve un cliente cuando alguien abre la conexión.
            WiFiClient incoming = server.available();
            if (incoming) {
                client = incoming;
                client.setNoDelay(true);
                Serial.printf("[WiFi TCP] cliente conectado desde %s\n", client.remoteIP().toString().c_str());
            }
        }
        return;
    }

    if (client) {
        closeClient();
    }
    stopServer();

    if (connectPending) {
        // millis() puede desbordarse; la resta sin signo sigue midiendo el
        // tiempo transcurrido correctamente en ese caso.
        if (static_cast<uint32_t>(now - connectStartedMs) >= kConnectAttemptTimeoutMs) {
            Serial.println("[WiFi TCP] tiempo de conexión agotado; se reintentará.");
            WiFi.disconnect(false, false);
            connectPending = false;
            lastConnectAttemptMs = now;
        }
        return;
    }

    if (static_cast<uint32_t>(now - lastConnectAttemptMs) >= kReconnectIntervalMs) {
        beginConnectAttempt(now);
    }
}

extern "C" bool hal_telemetry_network_send_line(const char *line) {
    if (line == nullptr || !client || !client.connected()) {
        return false;
    }

    // La red usa LF como separador. Rechazar líneas mezcladas o sin final
    // evita que el receptor una dos mensajes por error.
    size_t length = 0U;
    while (length <= kMaxLineBytes && line[length] != '\0') {
        if (line[length] == '\r' || (line[length] == '\n' && line[length + 1U] != '\0')) {
            return false;
        }
        ++length;
    }
    if (length == 0U || length > kMaxLineBytes || line[length - 1U] != '\n') {
        return false;
    }

    /*
     * Arduino-ESP32 WiFiClient::write() realiza reintentos acotados con
     * select() y send(MSG_DONTWAIT), pero puede devolver una escritura parcial.
     * En ese caso se cierra el socket para que una trama posterior no se
     * concatene con el fragmento incompleto.
     */
    const size_t written = client.write(reinterpret_cast<const uint8_t *>(line), length);
    if (written != length) {
        closeClient();
        Serial.println("[WiFi TCP] escritura incompleta; conexión cerrada.");
        return false;
    }
    return true;
}
