import processing.serial.*;

Serial myPort;
float theta = 0; // Ángulo recibido del ESP32 (en radianes)

void setup() {
  size(600, 600);
  
  println("Puertos serie disponibles:");
  printArray(Serial.list());
  
  // IMPORTANTE: Cambia "COM3" por el puerto exacto de tu ESP32 (Ej: "COM4", "/dev/ttyUSB0")
  String portName = "COM3"; 
  println("Intentando conectar a: " + portName);
  
  myPort = new Serial(this, portName, 115200);
  myPort.bufferUntil('\n'); 
}

void draw() {
  background(245); 
  
  drawGrid();
  
  pushMatrix();
  translate(width / 2, height / 2);
  
  rotate(-theta); 
  
  // Dibujo de la flecha azul
  fill(0, 102, 204); 
  stroke(0);        
  strokeWeight(2);
  triangle(-20, 20, -20, -20, 40, 0); 
  
  popMatrix(); 
  
  // Información en pantalla
  fill(0);
  textSize(16);
  text("Prueba Inercial MPU-6050", 20, 30);
  text("Pose: X = 0.0 mm | Y = 0.0 mm", 20, 55);
  text("Orientacion (Theta): " + nf(degrees(theta), 1, 2) + "°", 20, 80);
}

void serialEvent(Serial myPort) {
  try {
    String inString = myPort.readStringUntil('\n');
    if (inString != null) {
      inString = trim(inString); 
      if (inString.length() == 0 || inString.startsWith("#")) return;
      String[] data = split(inString, ',');
      
      if (data.length == 2) {
        theta = float(data[0]);
      }
    }
  } catch (Exception e) {
    // Evita que el programa crashee si entra basura por el puerto al reiniciar el ESP32
    println("Error de lectura o trama incompleta");
  }
}

void drawGrid() {
  stroke(220);
  strokeWeight(1);
  for (int i = 0; i < width; i += 40)  line(i, 0, i, height);
  for (int j = 0; j < height; j += 40) line(0, j, width, j);
}
