import processing.serial.*;

Serial myPort;
float theta = 0;
float distancia = 0;

// Lista dinámica de obstáculos
ArrayList<PVector> obstaculos = new ArrayList<PVector>();

void setup() {
  size(800, 800);
  
  // IMPORTANTE: Cambia "COM3" por tu puerto real
  String portName = "COM11"; 
  myPort = new Serial(this, portName, 115200);
}

void draw() {
  // 1. LEER EL PUERTO SERIE DE FORMA SEGURA EN EL HILO PRINCIPAL
  while (myPort.available() > 0) {
    String inString = myPort.readStringUntil('\n');
    if (inString != null) {
      inString = trim(inString); 
      String[] data = split(inString, ',');
      
      if (data.length == 2) {
        theta = float(data[0]);
        distancia = float(data[1]);
        
        // Si el ToF detecta un objeto válido (entre 5 cm y 150 cm)
        if (distancia > 5 && distancia < 150) {
          float escala = 3.0; // Píxeles por centímetro
          float distPx = distancia * escala;
          
          // Conversión de Polares a Cartesianas
          float xObs = distPx * cos(theta);
          float yObs = distPx * sin(theta);
          
          // Guardar el obstáculo en la lista de forma segura
          obstaculos.add(new PVector(xObs, -yObs));
          
          // Limitar la memoria a 400 puntos máximos
          if (obstaculos.size() > 400) {
            obstaculos.remove(0);
          }
        }
      }
    }
  }

  // 2. RENDERIZADO GRÁFICO
  background(30);
  drawGrid();

  pushMatrix();
  translate(width / 2, height / 2); // Centro de la pantalla

  // Dibujar puntos de obstáculos acumulados
  fill(255, 120, 0); // Naranja
  noStroke();
  for (PVector obs : obstaculos) {
    ellipse(obs.x, obs.y, 6, 6);
  }

  // Rotar el entorno según el ángulo del robot
  rotate(-theta);

  // Dibujar la flecha del robot
  fill(0, 102, 204); 
  stroke(255);        
  strokeWeight(2);
  triangle(-20, 20, -20, -20, 40, 0); 

  popMatrix(); 

  // 3. INTERFAZ DE TEXTO
  fill(255);
  textSize(16);
  text("Orientacion (Theta): " + nf(degrees(theta), 1, 2) + "°", 20, 30);
  text("Distancia ToF: " + (distancia > 0 ? distancia + " cm" : "Fuera de rango"), 20, 55);
  text("Obstáculos mapeados: " + obstaculos.size(), 20, 80);
}

void drawGrid() {
  stroke(50);
  strokeWeight(1);
  for (int i = 0; i < width; i += 40)  line(i, 0, i, height);
  for (int j = 0; j < height; j += 40) line(0, j, width, j);
}
