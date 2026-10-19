# Ensayo `wifi_tcp_only` — Arturito V0.2

## Objetivo y alcance

Este ensayo comprueba por separado el enlace Wi-Fi/TCP del ESP32: se conecta
como estación (STA) a un router existente, ofrece un servidor TCP para un único
cliente y publica un contador aproximadamente cada segundo. No inicializa
sensores, no envía comandos y no integra Processing ni `perception_v0`.

El ESP32 es aquí una plataforma de desarrollo y validación. La plataforma
objetivo sigue siendo EDU-CIAA; este ensayo no sustituye el diseño definitivo
EDU-CIAA más módulo de comunicaciones (NodeMCU), ni demuestra que ese enlace o
la integración final funcionen.

## Plataforma y archivos

Se utiliza ESP32 Dev Module con Arduino-ESP32 y PlatformIO. La funcionalidad de
red proviene del framework incluido con Arduino-ESP32; no se agrega una
biblioteca externa.

- `firmware/shared/layer1_hal/hal_telemetry_network.h`: contrato C de la HAL,
  sin tipos ni encabezados de Arduino.
- `firmware/ESP32/src/layer1_hal/hal_telemetry_network_esp32.cpp`: Wi-Fi STA,
  servidor, socket cliente, diagnósticos y reconexión específicos de ESP32.
- `firmware/ESP32/src/tests/wifi_tcp_only.cpp`: `setup()`/`loop()`, contador y
  salida de diagnóstico USB. No crea tareas FreeRTOS.
- `firmware/ESP32/include/wifi_secrets.example.h`: plantilla versionada sin
  credenciales reales.
- `firmware/ESP32/include/wifi_secrets.h`: copia privada local, ignorada por
  Git.
- `firmware/ESP32/platformio.ini`: entorno independiente `wifi_tcp_only`.

## Contratos HAL

- `hal_telemetry_network_start()` configura modo STA e inicia un intento de
  conexión sin esperar a que termine. Devuelve `true` si hay configuración y
  pudo iniciar el procedimiento, no cuando ya obtuvo conexión. Sin el archivo
  privado correctamente configurado, informa el problema por serie y devuelve
  `false`.
- `hal_telemetry_network_poll()` avanza la conexión y reconexión al router,
  inicia el servidor al conectarse, acepta un cliente y descarta clientes que
  se desconectan.
- `hal_telemetry_network_send_line(line)` acepta una sola línea ASCII terminada
  en `\n`, sin CR ni otras nuevas líneas, de hasta 127 bytes. Devuelve `false`
  si no hay cliente, el argumento no cumple el contrato o la escritura falla.

La implementación Arduino-ESP32 disponible hace que `WiFiClient::write()` use
esperas cortas de disponibilidad del socket y reintentos acotados, pero no
ofrece un contrato estricto de tiempo máximo de llamada. Si el envío devuelve
menos bytes que la longitud solicitada, la HAL cierra el socket; así el cliente
detectará fin de conexión y no interpretará una trama posterior como
continuación válida. Para este ensayo la mitigación es usar líneas pequeñas y
descartar los datos durante la desconexión. No se almacena historial ni se
garantiza entrega.

## Protocolo TCP

- ESP32: servidor TCP en el puerto configurable `HAL_TELEMETRY_TCP_PORT`, por
  defecto `3333`.
- Computadora: cliente en la misma red local que el ESP32.
- Cada trama es `COUNT,<contador>\n`, por ejemplo `COUNT,0` seguido del byte LF.
- TCP es un flujo de bytes; el cliente separa registros usando LF. La serie USB
  a 115200 baudios contiene diagnósticos y nunca se mezcla con los datos TCP.

El servidor admite un cliente a la vez. Tras su desconexión acepta uno nuevo.
Si se pierde el router, cierra el cliente y servidor; limita cada intento de
conexión a 15 s y espera al menos 5 s entre intentos. El bucle del ensayo sigue
atendiendo `poll()` y no espera indefinidamente por la red.

## Configurar credenciales

Desde `firmware/ESP32` en PowerShell:

```powershell
Copy-Item .\include\wifi_secrets.example.h .\include\wifi_secrets.h
```

Editá solo `include/wifi_secrets.h`: completá `ARTURITO_WIFI_SSID` y
`ARTURITO_WIFI_PASSWORD` con la red local y cambiá
`ARTURITO_WIFI_CONFIGURED` a `1`. Usá una red de 2.4 GHz compatible con el
módulo. El ejemplo mantiene `ARTURITO_WIFI_CONFIGURED` en `0`, así que copiarlo
sin configurar no inicia una conexión. No agregues el archivo privado al
repositorio. El monitor serie no muestra la contraseña.

## Compilar, cargar y observar

Desde `firmware/ESP32`:

```powershell
pio run -e wifi_tcp_only
pio device list
pio run -e wifi_tcp_only -t upload --upload-port COM5
pio device monitor -b 115200 --port COM5
```

Reemplazá `COM5` por el puerto que indique `pio device list`. El monitor debe
informar la conexión y la IP local, por ejemplo `192.168.1.42`, y luego que el
servidor escucha en el puerto 3333. La prueba de hardware corresponde a quien
disponga del ESP32, el router y la computadora; una compilación correcta no la
reemplaza.

## Recibir datos en Windows con PowerShell

Conectá la computadora y el ESP32 al mismo router. En PowerShell, reemplazá la
IP por la mostrada en el monitor serie y ejecutá:

```powershell
$ip = '192.168.1.42'
$port = 3333
$tcp = [System.Net.Sockets.TcpClient]::new()
$tcp.Connect($ip, $port)
$reader = [System.IO.StreamReader]::new($tcp.GetStream())
try {
    while ($tcp.Connected) {
        $line = $reader.ReadLine()
        if ($null -eq $line) { break }
        $line
    }
}
finally {
    $reader.Dispose()
    $tcp.Dispose()
}
```

Deberán aparecer líneas `COUNT,0`, `COUNT,1`, etc. `ReadLine()` quita el LF
usado como delimitador. Para detener la recepción, presioná Ctrl+C.

## Comprobar recuperación

1. Con la recepción en marcha, cerrá el cliente con Ctrl+C. El monitor serie
   debe informar la desconexión; el ensayo continúa y acepta la siguiente
   conexión TCP.
2. Volvé a ejecutar el cliente de PowerShell. Deben volver a recibirse líneas;
   los contadores generados mientras no había cliente se descartan.
3. Apagá temporalmente el router o alejás el ESP32 para perder cobertura. El
   monitor debe reportar el fallo/tiempo agotado y volver a intentar con pausa.
   Al recuperar el router, debe anunciar la IP y volver a aceptar clientes sin
   reiniciar el microcontrolador.

## Limitaciones

- La conectividad, el firewall de la computadora y el comportamiento ante
  pérdida de router requieren validación física.
- La escritura TCP tiene reintentos limitados en la biblioteca, no una garantía
  estricta de no bloqueo; una escritura parcial fuerza el cierre de la conexión.
- Solo se admite un cliente, sin almacenamiento ni reenvío de tramas perdidas.
- Esta prueba no define el protocolo definitivo entre EDU-CIAA y el módulo de
  comunicaciones.
