#include "config.h"
#include "hal_sensors.h"       
#include "driver_mpu6050.h"     
#include "driver_distancia.h"  

#if defined(BOARD_ESP32)
  #include <Arduino.h>
  #include <freertos/FreeRTOS.h>
  #include <freertos/task.h>
  #include <freertos/semphr.h>
#endif

// --- Estado compartido entre la tarea de sensores y el resto (capa 3/4) ---
struct SensorData {
  float theta_rad;
  float distancia_cm;
};

static SensorData g_sensor_data = {0, -1};
static SemaphoreHandle_t g_mutex;

void task_sensores(void* pv) {
  // Inicialización de HAL y Drivers (Nombres corregidos)
  hal_sensors_init();
  mpu6050_init();
  mpu6050_calibrate();
  distancia_init();

  while (true) {
    mpu6050_update();

    SensorData local;
    local.theta_rad = mpu6050_get_theta_rad();
    local.distancia_cm = distancia_get_cm();

    xSemaphoreTake(g_mutex, portMAX_DELAY);
    g_sensor_data = local;
    xSemaphoreGive(g_mutex);

    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
}

// Placeholder de lo que hoy hace capa 3/4
void task_publicar(void* pv) {
  while (true) {
    SensorData local;
    xSemaphoreTake(g_mutex, portMAX_DELAY);
    local = g_sensor_data;
    xSemaphoreGive(g_mutex);

    Serial.print(local.theta_rad, 4);
    Serial.print(",");
    Serial.println(local.distancia_cm);

    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
}

void setup() {
  Serial.begin(115200);

  g_mutex = xSemaphoreCreateMutex();

  xTaskCreate(task_sensores, "sensores", 4096, nullptr, 2, nullptr);
  xTaskCreate(task_publicar, "publicar", 2048, nullptr, 1, nullptr);
}

void loop() {
  // Vacío a propósito: todo corre en las tareas de FreeRTOS.
}