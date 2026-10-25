"""Conectar la PC a Arturito y ejecutar este receptor."""

import socket

try:
    print("Conectando a 192.168.4.1:3333...", flush=True)
    with socket.create_connection(("192.168.4.1", 3333), timeout=10) as connection:
        print("Conectado. Esperando datos...", flush=True)
        connection.settimeout(None)
        with connection.makefile("r", encoding="utf-8") as stream:
            for line in stream:
                print(line.rstrip("\r\n"), flush=True)
except KeyboardInterrupt:
    pass
except OSError as error:
    print(f"No se pudo comunicar con el ESP32: {error}")
