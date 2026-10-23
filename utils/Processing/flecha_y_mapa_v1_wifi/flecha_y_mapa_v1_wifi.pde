import processing.net.Client;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;

// IP predeterminada del AP creado por el ESP32.
final String ARTURITO_IP = "192.168.4.1";
final int ARTURITO_PORT = 3333;

// Los intentos tienen un timeout corto y se hacen en otro hilo para que
// draw() siga respondiendo aunque Arturito esté apagado.
final int CONNECT_TIMEOUT_MS = 1200;
final int RECONNECT_INTERVAL_MS = 3000;
final int DATA_TIMEOUT_MS = 2500;
final int MAX_LINE_LENGTH = 128;

volatile Client tcpClient = null;
volatile boolean connectionInProgress = false;
volatile boolean shuttingDown = false;

int lastConnectionAttemptMs = 0;
boolean attemptedConnection = false;
int lastSampleMs = 0;
boolean hasReceivedSample = false;
boolean receivedSampleOnCurrentConnection = false;

StringBuilder incomingLine = new StringBuilder();
boolean discardCurrentLine = false;

float theta = 0;
float distancia = 0;

// Nube polar acumulada en metros respecto del origen fijo del ensayo V0.
ArrayList<PVector> obstaculos = new ArrayList<PVector>();

void setup() {
  size(800, 800);
  surface.setTitle("Arturito V0.2 - Flecha y mapa por Wi-Fi/TCP");
}

void draw() {
  maintainConnection();
  readAvailableLines();

  background(30);
  drawGrid();

  pushMatrix();
  translate(width / 2, height / 2); // El origen sigue fijo en el centro.

  fill(255, 120, 0);
  noStroke();
  for (PVector obs : obstaculos) {
    ellipse(obs.x, obs.y, 6, 6);
  }

  rotate(-theta);

  fill(0, 102, 204);
  stroke(255);
  strokeWeight(2);
  triangle(-20, 20, -20, -20, 40, 0);

  popMatrix();

  drawStatus();
}

void maintainConnection() {
  Client current = tcpClient;
  if (current != null && !current.active()) {
    tcpClient = null;
    current.stop();
    incomingLine.setLength(0);
    discardCurrentLine = false;
    receivedSampleOnCurrentConnection = false;
  }

  int now = millis();
  boolean retryIsDue = !attemptedConnection ||
    (int)(now - lastConnectionAttemptMs) >= RECONNECT_INTERVAL_MS;

  if (tcpClient == null && !connectionInProgress && retryIsDue) {
    startConnectionAttempt(now);
  }
}

void startConnectionAttempt(final int attemptTimeMs) {
  attemptedConnection = true;
  lastConnectionAttemptMs = attemptTimeMs;
  connectionInProgress = true;

  Thread connector = new Thread(new Runnable() {
    public void run() {
      Socket socket = null;
      Client newClient = null;

      try {
        socket = new Socket();
        socket.connect(new InetSocketAddress(ARTURITO_IP, ARTURITO_PORT),
                       CONNECT_TIMEOUT_MS);
        newClient = new Client(flecha_y_mapa_v1_wifi.this, socket);
      } catch (IOException e) {
        if (socket != null) {
          try {
            socket.close();
          } catch (IOException ignored) {
            // No queda nada más que liberar en este intento.
          }
        }
      }

      if (newClient != null) {
        if (shuttingDown) {
          newClient.stop();
        } else {
          tcpClient = newClient;
        }
      }
      connectionInProgress = false;
    }
  });

  connector.setName("arturito-tcp-connect");
  connector.setDaemon(true);
  connector.start();
}

void readAvailableLines() {
  Client current = tcpClient;
  if (current == null || !current.active()) {
    return;
  }

  // TCP puede entregar media línea o varias juntas. El LF marca cada trama.
  while (current.available() > 0) {
    int nextByte = current.read();
    if (nextByte < 0) {
      break;
    }

    if (nextByte == '\n') {
      if (!discardCurrentLine) {
        acceptTelemetryLine(incomingLine.toString());
      }
      incomingLine.setLength(0);
      discardCurrentLine = false;
    } else if (!discardCurrentLine) {
      if (incomingLine.length() < MAX_LINE_LENGTH) {
        incomingLine.append((char)(nextByte & 0xff));
      } else {
        // Una trama desmedida se descarta hasta el siguiente LF.
        incomingLine.setLength(0);
        discardCurrentLine = true;
      }
    }
  }
}

void acceptTelemetryLine(String line) {
  String cleanLine = trim(line); // También quita un posible CR antes del LF.
  if (cleanLine.length() == 0) {
    return;
  }

  int comma = cleanLine.indexOf(',');
  if (comma <= 0 || comma != cleanLine.lastIndexOf(',') ||
      comma == cleanLine.length() - 1) {
    return;
  }

  float nextThetaDeg;
  float nextDistanceCm;
  try {
    nextThetaDeg = Float.parseFloat(trim(cleanLine.substring(0, comma)));
    nextDistanceCm = Float.parseFloat(trim(cleanLine.substring(comma + 1)));
  } catch (NumberFormatException e) {
    return;
  }

  if (Float.isNaN(nextThetaDeg) || Float.isInfinite(nextThetaDeg) ||
      Float.isNaN(nextDistanceCm) || Float.isInfinite(nextDistanceCm)) {
    return;
  }

  // Una trama válida actualiza el estado visible, aunque la distancia sea inválida.
  theta = radians(nextThetaDeg);
  distancia = nextDistanceCm / 100.0;
  lastSampleMs = millis();
  hasReceivedSample = true;
  receivedSampleOnCurrentConnection = true;

  // Se conserva el filtro, la escala y la proyección polar del sketch original.
  if (distancia >= 0.05 && distancia <= 1.50) {
    float pixelsPerMeter = 250.0;
    float distancePx = distancia * pixelsPerMeter;
    float xObs = distancePx * cos(theta);
    float yObs = distancePx * sin(theta);
    obstaculos.add(new PVector(xObs, -yObs));

    if (obstaculos.size() > 400) {
      obstaculos.remove(0);
    }
  }
}

String connectionStatus() {
  Client current = tcpClient;
  if (current != null && current.active()) {
    if (receivedSampleOnCurrentConnection &&
        (int)(millis() - lastSampleMs) <= DATA_TIMEOUT_MS) {
      return "Conectado / Recibiendo datos";
    }
    return "Conectado / Sin datos recientes";
  }
  if (connectionInProgress) {
    return "Conectando...";
  }
  if (attemptedConnection) {
    return "Desconectado / Reintentando";
  }
  return "Conectando...";
}

void drawStatus() {
  fill(255);
  textSize(16);
  if (hasReceivedSample) {
    text("Orientacion (Theta): " + nf(degrees(theta), 1, 2) + "°", 20, 30);
    text("Distancia ToF: " + (distancia > 0 ? nf(distancia, 1, 3) + " m" : "Fuera de rango"), 20, 55);
  } else {
    text("Orientacion (Theta): esperando datos", 20, 30);
    text("Distancia ToF: esperando datos", 20, 55);
  }
  text("Obstaculos mapeados: " + obstaculos.size(), 20, 80);

  textSize(14);
  text("TCP: " + connectionStatus(), 20, 108);
}

void drawGrid() {
  stroke(50);
  strokeWeight(1);
  for (int i = 0; i < width; i += 40) line(i, 0, i, height);
  for (int j = 0; j < height; j += 40) line(0, j, width, j);
}

void dispose() {
  shuttingDown = true;
  Client current = tcpClient;
  tcpClient = null;
  if (current != null) {
    current.stop();
  }
  super.dispose();
}
