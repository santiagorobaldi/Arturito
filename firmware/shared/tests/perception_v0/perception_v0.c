#include "perception_v0.h"

#include "config.h"
#include "driver_vl53l0x.h"
#include "driver_mpu6050.h"
#include "hal_telemetry.h"
#include "hal_time.h"
#if defined(PERCEPTION_V0_WIFI_TCP)
#include "hal_telemetry_network.h"
#endif

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

#include <math.h>

static bool initialized, has_previous;
static float angle, deadband, previous_velocity;
static uint32_t previous_time;

/** Establece el angulo inicial y la zona muerta en rad/s para este ensayo. */
static bool odometry_init(float initial_angle_rad, float deadband_rads) {
    if (deadband_rads < 0.0f) {
        return false;
    }
    angle = initial_angle_rad;
    deadband = deadband_rads;
    has_previous = false;
    initialized = true;
    return true;
}

/** Integra velocidades consecutivas por el metodo del trapecio.
 * La primera muestra fija la referencia temporal. Acepta el desbordamiento
 * del reloj y rechaza intervalos nulos o mayores o iguales a 2^31 ms. */
static bool odometry_update(float angular_velocity_rads, uint32_t sample_time_ms) {
    if (!initialized) {
        return false;
    }
    float velocity = fabsf(angular_velocity_rads) <= deadband ?
                     0.0f : angular_velocity_rads;
    if (has_previous) {
        uint32_t elapsed = sample_time_ms - previous_time;
        if (elapsed == 0U || elapsed >= 0x80000000U) {
            return false;
        }
        double dt_s = (double)elapsed / 1000.0;
        double average_velocity = ((double)previous_velocity + velocity) / 2.0;
        angle += (float)(average_velocity * dt_s);
    }
    previous_velocity = velocity;
    previous_time = sample_time_ms;
    has_previous = true;
    return true;
}

static float odometry_get_angle(void) { return angle; }

static bool odometry_reset(float angle_rad) {
    if (!initialized) {
        return false;
    }
    angle = angle_rad;
    has_previous = false;
    return true;
}

/** Descarta la muestra anterior y conserva el angulo acumulado. */
static bool odometry_restart_tracking(void) {
    return odometry_reset(odometry_get_angle());
}

#define APP_START_STACK_BYTES       4096U
#define APP_ACQUIRE_STACK_BYTES     4096U
#define APP_DRIVERS_STACK_BYTES     4096U
#if defined(PERCEPTION_V0_WIFI_TCP)
/* La publicación también llama a Wi-Fi/Arduino, que necesita más pila. */
#define APP_PUBLISH_STACK_BYTES     4096U
#else
#define APP_PUBLISH_STACK_BYTES     2048U
#endif

#define APP_START_PRIORITY          4U
#define APP_ACQUIRE_PRIORITY        3U
#define APP_DRIVERS_PRIORITY        2U
#define APP_PUBLISH_PRIORITY        1U

typedef struct {
    float gyro_z_rads;
    float distancia_m_raw;
    uint32_t time_ms;
    uint32_t sequence;
    bool imu_valid;
} RawSample;

typedef struct {
    float theta_rad;
    float distancia_m;
} AppSample;

static RawSample raw_sample = {0};
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
    uint32_t sequence = 0;

    for (;;) {
        RawSample sample;
        mpu6050_sample_t imu;
        sample.imu_valid = mpu6050_read(&imu);
        sample.time_ms = hal_millis();
        sample.sequence = ++sequence;
        sample.gyro_z_rads = sample.imu_valid ? imu.gyro_z_rads : 0.0f;
        sample.distancia_m_raw = vl53l0x_get_distance_m();

        xSemaphoreTake(raw_mutex, portMAX_DELAY);
        raw_sample = sample;
        xSemaphoreGive(raw_mutex);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

static void task_process_sample(void *context) {
    (void)context;
    uint32_t previous_sequence = 0;

    for (;;) {
        RawSample raw;
        xSemaphoreTake(raw_mutex, portMAX_DELAY);
        raw = raw_sample;
        xSemaphoreGive(raw_mutex);

        if (raw.sequence == previous_sequence) {
            vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
            continue;
        }
        if (raw.sequence - previous_sequence != 1U || !raw.imu_valid) {
            odometry_restart_tracking();
        }
        previous_sequence = raw.sequence;
        if (raw.imu_valid && !odometry_update(raw.gyro_z_rads, raw.time_ms)) {
            odometry_restart_tracking();
        }

        AppSample sample;
        sample.theta_rad = odometry_get_angle();
        sample.distancia_m = raw.distancia_m_raw;

        xSemaphoreTake(app_mutex, portMAX_DELAY);
        app_sample = sample;
        xSemaphoreGive(app_mutex);

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

static void task_publish(void *context) {
    (void)context;

#if defined(PERCEPTION_V0_WIFI_TCP)
    hal_telemetry_write("# DEBUG: WIFI COMPILADO");
#else
    hal_telemetry_write("# DEBUG: WIFI NO COMPILADO");
#endif

    // El resto de la función queda exactamente igual.

#if defined(PERCEPTION_V0_WIFI_TCP)
    /* Esta tarea es la única que toca el cliente TCP durante todo el test. */
    const bool network_started = hal_telemetry_network_start();
    if (!network_started) {
        hal_telemetry_write("# WARN: no se inició Wi-Fi; continúa la salida serie.");
    }
#endif

    for (;;) {
        AppSample sample;
        char line[48];

        xSemaphoreTake(app_mutex, portMAX_DELAY);
        sample = app_sample;
        xSemaphoreGive(app_mutex);

        snprintf(line, sizeof(line), "%.4f,%.4f", sample.theta_rad,
                 sample.distancia_m);
        hal_telemetry_write(line);

#if defined(PERCEPTION_V0_WIFI_TCP)
        /* Los diagnósticos siguen por USB. TCP recibe únicamente el CSV. */
        hal_telemetry_network_poll();
        char tcp_line[sizeof(line) + 1U];
        snprintf(tcp_line, sizeof(tcp_line), "%s\n", line);
        (void)hal_telemetry_network_send_line(tcp_line);
#endif

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
    }
}

static void delete_task_if_created(TaskHandle_t task) {
    if (task != NULL) {
        vTaskDelete(task);
    }
}

static void task_start_test(void *context) {
    (void)context;
    hal_telemetry_write("# Arturito protocolo V0: theta_rad,distancia_m");

    if (!mpu6050_init(IMU_I2C_ADDR)) {
        hal_telemetry_write("# ERROR: IMU MPU6050 no encontrada. Halt.");
        vTaskDelete(NULL);
    }
    hal_telemetry_write("# IMU OK");

    if (!vl53l0x_init()) {
        hal_telemetry_write("# WARN: VL53L0X no encontrado. distancia_m=-1");
    } else {
        hal_telemetry_write("# ToF OK");
    }

    hal_telemetry_write("# Calibrando IMU (quedate quieto)...");
    hal_delay(2000);
    if (!mpu6050_calibrate_gyro(1000U, 2U) ||
        !odometry_init(0.0f, 0.017453292519943295f)) {
        hal_telemetry_write("# ERROR: fallo calibracion/inicio de orientacion.");
        vTaskDelete(NULL);
        return;
    }
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
        created = xTaskCreate(task_process_sample, "drivers",
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

bool perception_v0_start(void) {
    raw_sample = (RawSample){0};
    raw_sample.distancia_m_raw = -1.0f;
    app_sample.theta_rad = 0.0f;
    app_sample.distancia_m = -1.0f;

    return xTaskCreate(task_start_test, "app_start",
                       stack_depth_from_bytes(APP_START_STACK_BYTES), NULL,
                       APP_START_PRIORITY, NULL) == pdPASS;
}
