import processing.serial.*;

Serial myPort;
float theta = 0;
float distancia = 0;

// Nube polar acumulada en metros respecto del origen.
ArrayList<PVector> obstaculos = new ArrayList<PVector>();

void setup() {
  size(800, 800);
  
  // IMPORTANTE: Cambia "COM3" por tu puerto real
  String portName = "COM3";
  myPort = new Serial(this, portName, 115200);
}

void draw() {
  // 1. CSV V0: theta_rad,distancia_m; las lineas # son logs.
  while (myPort.available() > 0) {
    String inString = myPort.readStringUntil('\n');
    if (inString != null) {
      inString = trim(inString);
      if (inString.length() == 0 || inString.startsWith("#")) {
        continue;
      }
      String[] data = split(inString, ',');

      if (data.length == 2) {
        float nextTheta = float(data[0]);
        float nextDistance = float(data[1]);
        if (Float.isNaN(nextTheta) || Float.isInfinite(nextTheta) ||
            Float.isNaN(nextDistance) || Float.isInfinite(nextDistance)) {
          continue;
        }

        theta = nextTheta;
        distancia = nextDistance;

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
    }
  }

  background(30);
  drawGrid();

  pushMatrix();
  translate(width / 2, height / 2); // Centro de la pantalla

  fill(255, 120, 0); // Naranja
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

  fill(255);
  textSize(16);
  text("Orientacion (Theta): " + nf(degrees(theta), 1, 2) + "°", 20, 30);
  text("Distancia ToF: " + (distancia > 0 ? nf(distancia, 1, 3) + " m" : "Fuera de rango"), 20, 55);
  text("Obstáculos mapeados: " + obstaculos.size(), 20, 80);
}

void drawGrid() {
  stroke(50);
  strokeWeight(1);
  for (int i = 0; i < width; i += 40)  line(i, 0, i, height);
  for (int j = 0; j < height; j += 40) line(0, j, width, j);
}
