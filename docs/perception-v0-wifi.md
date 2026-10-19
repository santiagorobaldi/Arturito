# Ensayo `perception_v0_wifi` — Arturito V0.2

## Objetivo

Este entorno reutiliza el ensayo V0 de percepción y publica la misma muestra
procesada por los dos transportes disponibles:

- USB serie a 115200 baudios, con los diagnósticos y el CSV habituales de V0.
- Wi-Fi/TCP, con solo líneas `theta_rad,distancia_m` y LF (`\n`) al final.

Los valores conservan cuatro decimales y las unidades de V0. Una distancia
inválida sigue apareciendo como `-1.0000`. No se integran sensores nuevos,
Processing, posición XY ni navegación. EDU-CIAA continúa siendo la plataforma
objetivo; esta compilación es un ensayo en ESP32.

## Cómo se organiza

El entorno `perception_v0_wifi` parte de `perception_v0`: conserva sus mismos
sensores, drivers, cálculo angular, tareas y dependencias, y añade el backend de
red con la opción `PERCEPTION_V0_WIFI_TCP`. La opción solo afecta a este entorno;
`perception_v0`, `esp32dev`, `wifi_tcp_only`, `imu_only` y `vl53l0x_only` mantienen
sus filtros de fuentes y comportamiento.

La aplicación forma el CSV una sola vez desde la última muestra procesada. Lo
envía por serie como antes y agrega el LF requerido por TCP antes de pasarlo a
la HAL de red. Los mensajes de inicialización y diagnóstico continúan por USB y
nunca se mandan al cliente TCP.

La tarea `publicar` es la única propietaria de `start()`, `poll()` y `send_line()`
en este entorno. La adquisición y el procesamiento no acceden a objetos Wi-Fi y
siguen siendo las tareas de mayor prioridad. La red se atiende fuera de los
mutex que protegen las muestras. La pila de `publicar` aumenta de 2048 a 4096
bytes solo en esta compilación para dar margen a las llamadas del framework
Arduino-ESP32; el tamaño usado en EDU-CIAA y en el V0 original no cambia.

El período de 30 ms es nominal. Incluye la atención de red y el tiempo de
ejecución, por lo que no representa una frecuencia garantizada. Si no hay
cliente, si se pierde el router o si falla un envío, el CSV por serie y el
procesamiento siguen activos; las muestras sin cliente se descartan. La
biblioteca limita los intentos de escritura, aunque no garantiza un tiempo
máximo estricto para cada llamada.

## Preparación en Windows / VS Code

1. En VS Code abrí el workspace de Arturito y la terminal integrada.
2. Cambiá al proyecto PlatformIO del ESP32:

   ```powershell
   Set-Location C:\Arturito\firmware\ESP32
   ```

3. Revisá que `include\wifi_secrets.h` contenga tu configuración local. Ese
   archivo está excluido de Git; no copies sus valores a la plantilla ni a la
   documentación.
4. Comprobá qué puerto COM usa la placa y compilá el entorno:

   ```powershell
   pio device list
   pio run -e perception_v0_wifi
   ```

5. Cargá el firmware y abrí el monitor serie, reemplazando `COM5` por el puerto
   mostrado en tu equipo:

   ```powershell
   pio run -e perception_v0_wifi -t upload --upload-port COM5
   pio device monitor -b 115200 --port COM5
   ```

También podés seleccionar `perception_v0_wifi` en **PlatformIO: Project Tasks**
de VS Code y usar las acciones **Build**, **Upload** y **Monitor**.

Al completar la inicialización y calibración, el monitor conserva los mensajes
de V0 y muestra líneas CSV como `0.1234,0.4567`. Si falla la IMU, la aplicación
termina como en V0 y no inicia la publicación Wi-Fi. Si la distancia no está
disponible, el segundo valor será `-1.0000`.

## Recibir el CSV por TCP

El ESP32 inicia el servidor al conectarse al router y muestra la IP local en el
monitor serie. Conectá la computadora al mismo router. Abrí otra terminal
PowerShell y reemplazá la dirección de ejemplo:

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

La salida TCP debe contener únicamente pares separados por coma, por ejemplo
`0.1234,0.4567`. `ReadLine()` consume el LF delimitador. Ctrl+C cierra el
cliente; el ESP32 debe seguir adquiriendo y publicando por serie y aceptar una
nueva conexión. La recuperación después de apagar el router aún requiere una
prueba física específica.

## Evidencia y pendiente

La prueba física anterior de `wifi_tcp_only` comprobó el modo STA, recepción de
`COUNT` y reconexión del cliente con el contador en marcha. No comprobó la caída
y recuperación del router. Este entorno integrado queda **implementado** tras
compilarse, pero su combinación de sensores reales con recepción CSV TCP debe
probarse en el ESP32 antes de considerarla validada.
