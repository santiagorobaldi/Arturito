# Visualizador Processing por Wi-Fi/TCP — Arturito V0.2

## Alcance

`utils/Processing/flecha_y_mapa_v1_wifi/flecha_y_mapa_v1_wifi.pde` es una
variante independiente del visualizador serie
`utils/Processing/flecha_y_mapa_v1/flecha_y_mapa_v1.pde`. Mantiene el tamaño de
ventana, la grilla, la flecha, el origen fijo, la escala de 250 píxeles por
metro y el historial de hasta 400 puntos polares. No calcula posición XY ni
implementa una grilla de ocupación: es la visualización relativa del ensayo V0.

## Preparación

1. Abrí Processing.
2. Elegí **File → Open** y abrí `flecha_y_mapa_v1_wifi.pde` dentro de su carpeta.
3. Si cambió la dirección asignada por el router, editá al comienzo del sketch:

   ```java
   final String ARTURITO_IP = "192.168.0.172";
   final int ARTURITO_PORT = 3333;
   ```

   La IP puede cambiar si el router no mantiene una reserva DHCP para el ESP32.
   La computadora y Arturito deben estar conectados a la misma red local.

4. Cargá y ejecutá el entorno `perception_v0_wifi` desde PlatformIO. Para los
   comandos de compilación y carga, consultá
   [perception-v0-wifi.md](perception-v0-wifi.md). Si usás el monitor serie,
   configurá `monitor_dtr = 0` y `monitor_rts = 0` en el entorno para evitar que
   sus líneas de control reinicien la placa al abrirlo.
5. Alimentá el prototipo como en la prueba validada. También puede funcionar
   con alimentación externa y sin el USB conectado a la computadora.
6. Comprobá en el router que la MAC del ESP32 esté conectada y anotá la IP
   actual. Si no aparece, revisá alimentación, reiniciá la placa y confirmá que
   el entorno cargado sea `perception_v0_wifi`.
7. Cerrá cualquier cliente TCP que ya esté usando el puerto, incluido el
   script de PowerShell. El servidor del ESP32 acepta un solo cliente a la vez;
   cerrá esa conexión antes de ejecutar Processing.
8. Ejecutá el sketch. Puede iniciarse antes que el ESP32: intentará conectar y
   volverá a probar cada 3 segundos.

## Datos y reconexión

El cliente recibe líneas `theta_rad,distancia_m\n`. TCP entrega un flujo de
bytes, así que el sketch acumula los bytes hasta LF: una línea puede llegar en
varias lecturas, y una lectura puede contener varias líneas. Se admite CR antes
de LF. Cada línea debe tener exactamente dos números finitos; las líneas
vacías, demasiado largas, incompletas o mal formadas se descartan sin detener
la ventana.

La conexión se intenta en un hilo separado, con timeout de 1,2 segundos. Así,
un intento fallido no congela el dibujo. El siguiente intento se agenda cada 3
segundos, nunca en cada frame. El estado en pantalla distingue entre intento de
conexión, conexión sin muestras recientes, recepción activa y desconexión con
reintento. Una distancia inválida como `-1.0000` actualiza la última lectura,
pero no agrega un obstáculo. Los puntos anteriores se conservan al desconectar.

## Prueba y diagnóstico

- **Conectado / Recibiendo datos:** verificá que cambien la orientación o la
  distancia al mover el prototipo y que crezca el contador de obstáculos cuando
  la lectura ToF esté entre 0,05 m y 1,50 m.
- **Conectado / Sin datos recientes:** el socket está abierto, pero no llegó
  una muestra válida en los últimos 2,5 segundos. Confirmá que esté corriendo
  `perception_v0_wifi`.
- **Desconectado / Reintentando:** revisá la IP y el puerto, que ambos equipos
  estén en la misma red y que no haya otro cliente TCP conectado. Cerrá el
  cliente anterior y reiniciá el servidor ESP32 si hace falta.
- Para comprobar la reconexión, detené el sketch o desconectá temporalmente la
  red del ESP32; luego volvé a ejecutar/conectar el firmware. Processing debe
  seguir dibujando durante la desconexión y reconectar al volver el servidor.

La recuperación del ESP32 ante una caída del router todavía requiere una
prueba física específica. La reconexión de Processing solo cubre el lado
cliente cuando el servidor vuelve a estar disponible.

## Limitaciones

La flecha y la nube conservan el comportamiento geométrico de V0. El origen no
se mueve y los puntos no se transforman según desplazamientos del rover. Esto
no es odometría cartesiana, raycasting, una grilla de ocupación ni SLAM. La
integración de este sketch con el ESP32 y sensores reales queda pendiente de
validación física por el usuario.
