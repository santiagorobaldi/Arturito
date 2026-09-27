import processing.serial.*;

Serial myPort;
float theta = 0; // Ángulo recibido de la EDU-CIAA (en radianes)

void setup() {
  size(600, 600); // Tamaño de la ventana en píxeles (600x600)
  
  // Imprimir en la consola inferior los puertos COM disponibles
  println("Puertos serie disponibles:");
  println(Serial.list());
  
  // ATENCIÓN: Si tu EDU-CIAA está en el primer puerto de la lista, usa [0].
  // Si aparece en el segundo o tercero, cambia el índice a [1], [2], etc.
  String portName = Serial.list()[0]; 
  println("Conectando a: " + portName);
  
  // Abrir puerto serie a 115200 baudios (misma velocidad que la EDU-CIAA)
  myPort = new Serial(this, portName, 115200);
  myPort.bufferUntil('\n'); // Esperar a recibir una línea completa
}

void draw() {
  background(245); // Fondo gris claro
  
  // Dibujar Grilla de referencia 2D
  drawGrid();
  
  // Movemos el origen (0,0) del dibujo al centro exacto de la ventana (300, 300)
  pushMatrix();
  translate(width / 2, height / 2);
  
  // Rotamos el plano según el ángulo recibido por la MPU
  // El signo negativo se debe a que en pantallas la Y crece hacia abajo
  rotate(-theta); 
  
  // --- DIBUJO DE LA FLECHA / FLECHA ROBA-POSICIÓN ("ARTURITO") ---
  fill(0, 102, 204); // Relleno azul
  stroke(0);        // Borde negro
  strokeWeight(2);
  
  // Dibujamos un triángulo/flecha con la punta orientada hacia la derecha (0 rad)
  triangle(-20, 20, -20, -20, 40, 0); 
  
  popMatrix(); // Restauramos la matriz de dibujo original
  
  // --- INFORMACIÓN EN PANTALLA ---
  fill(0);
  textSize(16);
  text("Prueba Inercial MPU-6050 (EDU-CIAA + FreeRTOS)", 20, 30);
  text("Pose: X = 0.0 mm | Y = 0.0 mm", 20, 55);
  text("Orientacion (Theta): " + nf(degrees(theta), 1, 2) + "°", 20, 80);
}

// Evento automático que se dispara cada vez que la EDU-CIAA manda un "\n"
void serialEvent(Serial myPort) {
  String inString = myPort.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString); // Eliminar espacios o saltos de línea extra
    String[] data = split(inString, ',');
    
    // Verificamos que lleguen las 3 variables "x,y,theta"
    if (data.length == 3) {
      theta = float(data[2]); // Extraemos únicamente el tercer valor (theta)
    }
  }
}

// Función auxiliar para dibujar la grilla 2D de fondo
void drawGrid() {
  stroke(220);
  strokeWeight(1);
  for (int i = 0; i < width; i += 40)  line(i, 0, i, height);
  for (int j = 0; j < height; j += 40) line(0, j, width, j);
}
