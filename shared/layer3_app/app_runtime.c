#include "app_runtime.h"

#include "config.h"
#include "driver_distancia.h"
#include "driver_mpu6050.h"
#include "hal_imu.h"
#include "hal_telemetry.h"
#include "hal_time.h"
#include "hal_tof.h"

#include <stdio.h>
#include <stdint.h>

#if defined(BOARD_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#else
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#endif

#define APP_START_STACK_BYTES       4096U
#define APP_ACQUIRE_STACK_BYTES     4096U
#define APP_DRIVERS_STACK_BYTES     4096U
#define APP_PUBLISH_STACK_BYTES     2048U

#define APP_START_PRIORITY          4U
#define APP_ACQUIRE_PRIORITY        3U
#define APP_DRIVERS_PRIORITY        2U
#define APP_PUBLISH_PRIORITY        1U

typedef struct {
    float gyro_z_rads;
    float distancia_m_raw;
} RawSample;

typedef struct {
    float theta_rad;
    float distancia_m;
} AppSample;

static RawSample raw_sample = {0.0f, -1.0f};
static AppSample app_sample = {0.0f, -1.0f};
static SemaphoreHandle_t raw_mutex;
static SemaphoreHandle_t app_mutex;

static uint32_t stack_depth_from_bytes(uint32_t stack_bytes) {
#if defined(BOARD_ESP32)
    return stack_bytes;
#else
    return (stack_bytes + sizeof(StackType_t) - 1U) / sizeof(StackType_t);
#endif
}

static void task_acquire(void *context) {
    (void)context;

    for (;;) {
        RawSample sample;
        sample.gyro_z_rads = hal_imu_get_gyro_z_rads();
        sample.distancia_m_raw = hal_tof_get_distance_m();

        xSemaphoreTake(raw_mutex, portMAX_DELAY);
        raw_sample = sample;
        xSemaphoreGive(raw_mutex);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

static void task_update_drivers(void *context) {
    (void)context;

    for (;;) {
        RawSample raw;
        xSemaphoreTake(raw_mutex, portMAX_DELAY);
        raw = raw_sample;
        xSemaphoreGive(raw_mutex);

        mpu6050_update_gyro(raw.gyro_z_rads);

        AppSample sample;
        sample.theta_rad = mpu6050_get_theta_rad();
        sample.distancia_m = distancia_validate_m(raw.distancia_m_raw);

        xSemaphoreTake(app_mutex, portMAX_DELAY);
        app_sample = sample;
        xSemaphoreGive(app_mutex);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

static void task_publish(void *context) {
    (void)context;

    for (;;) {
        AppSample sample;
        char line[48];

        xSemaphoreTake(app_mutex, portMAX_DELAY);
        sample = app_sample;
        xSemaphoreGive(app_mutex);

        snprintf(line, sizeof(line), "%.4f,%.4f", sample.theta_rad,
                 sample.distancia_m);
        hal_telemetry_write(line);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

static void delete_task_if_created(TaskHandle_t task) {
    if (task != NULL) {
        vTaskDelete(task);
    }
}

static void task_start_application(void *context) {
    (void)context;
    hal_telemetry_write("# Arturito protocolo V0: theta_rad,distancia_m");

    if (!hal_imu_init()) {
        hal_telemetry_write("# ERROR: IMU MPU6050 no encontrada. Halt.");
        vTaskDelete(NULL);
    }
    hal_telemetry_write("# IMU OK");

    if (!hal_tof_init()) {
        hal_telemetry_write("# WARN: VL53L0X no encontrado. distancia_m=-1");
    } else {
        hal_telemetry_write("# ToF OK");
    }

    mpu6050_init();
    hal_telemetry_write("# Calibrando IMU (quedate quieto)...");
    mpu6050_calibrate();
    distancia_init();
    hal_telemetry_write("# Calibracion OK. Datos CSV:");

    raw_mutex = xSemaphoreCreateMutex();
    app_mutex = xSemaphoreCreateMutex();
    if (raw_mutex == NULL || app_mutex == NULL) {
        hal_telemetry_write("# ERROR: no se pudieron crear los mutexes.");
        if (raw_mutex != NULL) {
            vSemaphoreDelete(raw_mutex);
            raw_mutex = NULL;
        }
        if (app_mutex != NULL) {
            vSemaphoreDelete(app_mutex);
            app_mutex = NULL;
        }
        vTaskDelete(NULL);
    }

    TaskHandle_t acquire_task = NULL;
    TaskHandle_t drivers_task = NULL;
    TaskHandle_t publish_task = NULL;

    BaseType_t created = xTaskCreate(task_acquire, "adquirir",
                                     stack_depth_from_bytes(APP_ACQUIRE_STACK_BYTES),
                                     NULL, APP_ACQUIRE_PRIORITY, &acquire_task);
    if (created == pdPASS) {
        created = xTaskCreate(task_update_drivers, "drivers",
                              stack_depth_from_bytes(APP_DRIVERS_STACK_BYTES),
                              NULL, APP_DRIVERS_PRIORITY, &drivers_task);
    }
    if (created == pdPASS) {
        created = xTaskCreate(task_publish, "publicar",
                              stack_depth_from_bytes(APP_PUBLISH_STACK_BYTES),
                              NULL, APP_PUBLISH_PRIORITY, &publish_task);
    }

    if (created != pdPASS) {
        hal_telemetry_write("# ERROR: no se pudieron crear las tareas.");
        delete_task_if_created(acquire_task);
        delete_task_if_created(drivers_task);
        delete_task_if_created(publish_task);
        vSemaphoreDelete(raw_mutex);
        vSemaphoreDelete(app_mutex);
        raw_mutex = NULL;
        app_mutex = NULL;
    }

    vTaskDelete(NULL);
}

bool app_runtime_start(void) {
    raw_sample.gyro_z_rads = 0.0f;
    raw_sample.distancia_m_raw = -1.0f;
    app_sample.theta_rad = 0.0f;
    app_sample.distancia_m = -1.0f;

    return xTaskCreate(task_start_application, "app_start",
                       stack_depth_from_bytes(APP_START_STACK_BYTES), NULL,
                       APP_START_PRIORITY, NULL) == pdPASS;
}