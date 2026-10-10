import processing.serial.*;
import java.util.ArrayList;

Serial imuPort;
String PORT_NAME = "COM3"; // Cambialo por el puerto que muestra Serial.list().
final int BAUD_RATE = 115200;

ArrayList<Float> rawTheta = new ArrayList<Float>();
ArrayList<Float> processedTheta = new ArrayList<Float>();
ArrayList<Float> sampleTimes = new ArrayList<Float>();
int startedAt = -1;
String status = "Esperando datos CSV";

void setup() {
  size(1000, 800);
  surface.setTitle("MPU6050: theta crudo vs capa 2");
  println("Puertos serie disponibles:");
  printArray(Serial.list());
  println("Conectando a " + PORT_NAME + " a " + BAUD_RATE + " baudios");
  imuPort = new Serial(this, PORT_NAME, BAUD_RATE);
  imuPort.bufferUntil('\n');
}

void draw() {
  background(250);
  drawHeader();
  drawPlot();
}

void drawHeader() {
  fill(25);
  textAlign(LEFT, TOP);
  textSize(20);
  text("MPU6050: angulo crudo vs procesado", 24, 18);
  textSize(13);
  fill(70);
  // Cambiamos (rad) por (deg)
  text("Tiempo desde el primer dato     Arriba: diferencia de angulo (deg)     Abajo: ambas lecturas (deg)", 24, 48);
  fill(190, 65, 45);
  text("Angulo crudo integrado", 24, 73);
  fill(35, 105, 205);
  text("Angulo capa 2 (calibracion + deadband + integracion trapezoidal)", 165, 73);
  fill(90);
  textAlign(RIGHT, TOP);
  text(status, width - 24, 18);
}

synchronized void drawPlot() {
  int left = 78;
  int right = width - 28;
  int diffTop = 122;
  int diffBottom = 405;
  int thetaTop = 465;
  int thetaBottom = height - 65;
float minTheta = -10.0;
  float maxTheta = 10.0;
  float minDiff = -2.0;
  float maxDiff = 2.0;
  float elapsed = startedAt < 0 ? 0 : (millis() - startedAt) / 1000.0;
  float axisMax = max(10.0, elapsed);
  int visibleCount = 0;

  for (int i = 0; i < sampleTimes.size(); i++) {
    minTheta = min(minTheta, min(rawTheta.get(i), processedTheta.get(i)));
    maxTheta = max(maxTheta, max(rawTheta.get(i), processedTheta.get(i)));
    float difference = processedTheta.get(i) - rawTheta.get(i);
    minDiff = min(minDiff, difference);
    maxDiff = max(maxDiff, difference);
    visibleCount++;
  }
  float thetaPad = max(0.05, (maxTheta - minTheta) * 0.12);
  minTheta -= thetaPad;
  maxTheta += thetaPad;
  float diffPad = max(0.005, (maxDiff - minDiff) * 0.12);
  minDiff -= diffPad;
  maxDiff += diffPad;

  stroke(220);
  strokeWeight(1);
  textSize(11);
  textAlign(RIGHT, CENTER);
  for (int tick = 0; tick <= 6; tick++) {
    float diffY = map(tick, 0, 6, diffBottom, diffTop);
    float thetaY = map(tick, 0, 6, thetaBottom, thetaTop);
    line(left, diffY, right, diffY);
    line(left, thetaY, right, thetaY);
    fill(90);
    text(nf(lerp(minDiff, maxDiff, 1.0 - tick / 6.0), 1, 3), left - 9, diffY);
    text(nf(lerp(minTheta, maxTheta, 1.0 - tick / 6.0), 1, 2), left - 9, thetaY);
  }
  textAlign(CENTER, TOP);
  for (int tick = 0; tick <= 5; tick++) {
    float x = map(tick, 0, 5, left, right);
    line(x, diffTop, x, diffBottom);
    line(x, thetaTop, x, thetaBottom);
    fill(90);
    text(nf(axisMax * tick / 5.0, 1, 1) + "s", x, thetaBottom + 10);
  }
  stroke(45);
  line(left, diffTop, left, diffBottom);
  line(left, thetaTop, left, thetaBottom);
  line(left, diffBottom, right, diffBottom);
  line(left, thetaBottom, right, thetaBottom);
  float zeroY = map(0, minDiff, maxDiff, diffBottom, diffTop);
  stroke(130);
  strokeWeight(2);
  line(left, zeroY, right, zeroY);

  fill(45);
  textAlign(LEFT, TOP);
text("Diferencia: capa 2 - crudo (deg)", left, 102);
  text("Angulo theta (deg)", left, 445);

  drawDifference(sampleTimes, rawTheta, processedTheta, axisMax, left, right,
                 diffTop, diffBottom, minDiff, maxDiff, color(110, 75, 180));
  drawSeries(rawTheta, sampleTimes, axisMax, left, right, thetaTop, thetaBottom, minTheta, maxTheta,
             color(190, 65, 45));
  drawSeries(processedTheta, sampleTimes, axisMax, left, right, thetaTop, thetaBottom, minTheta, maxTheta,
             color(35, 105, 205));

  fill(70);
  textAlign(LEFT, BOTTOM);
  text("Muestras visibles: " + visibleCount, left, height - 10);
}

void drawDifference(ArrayList<Float> times, ArrayList<Float> raw, ArrayList<Float> processed,
                    float axisMax, int left, int right, int top, int bottom,
                    float minY, float maxY, int seriesColor) {
  noFill();
  stroke(seriesColor);
  strokeWeight(2);
  beginShape();
  for (int i = 0; i < times.size(); i++) {
    float x = map(times.get(i), 0, axisMax, left, right);
    float difference = processed.get(i) - raw.get(i);
    float y = map(difference, minY, maxY, bottom, top);
    vertex(x, y);
  }
  endShape();
}

void drawSeries(ArrayList<Float> values, ArrayList<Float> times, float axisMax,
                int left, int right, int top, int bottom, float minY, float maxY,
                int seriesColor) {
  noFill();
  stroke(seriesColor);
  strokeWeight(2);
  beginShape();
  for (int i = 0; i < values.size(); i++) {
    float x = map(times.get(i), 0, axisMax, left, right);
    float y = map(values.get(i), minY, maxY, bottom, top);
    vertex(x, y);
  }
  endShape();
}

synchronized void serialEvent(Serial port) {
  String line = port.readStringUntil('\n');
  if (line == null) return;
  line = trim(line);
  if (line.length() == 0) return;
  if (line.startsWith("#")) {
    status = line;
    return;
  }

  String[] fields = split(line, ',');
  if (fields.length != 3) return;
  try {
    // Convertimos las lecturas de radianes a grados al momento de guardarlas
    float raw = degrees(Float.parseFloat(trim(fields[1])));
    float processed = degrees(Float.parseFloat(trim(fields[2])));
    
    if (startedAt < 0) startedAt = millis();
    rawTheta.add(raw);
    processedTheta.add(processed);
    sampleTimes.add((millis() - startedAt) / 1000.0);
    status = "Recibiendo datos";
  } catch (Exception e) {
    status = "Trama invalida: " + line;
  }
}
