#include "config.h"
#include "driver_mpu6050.h"

#include <Arduino.h>

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

void setup() {
    Serial.begin(115200);
    Serial.println("# test imu_only: theta_rad");

    if (!mpu6050_init(IMU_I2C_ADDR)) {
        Serial.println("# ERROR: IMU no encontrada. Halt.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("# Calibrando IMU (quedate quieto)...");
    delay(2000);
    if (!mpu6050_calibrate_gyro(1000U, 2U) ||
        !odometry_init(0.0f, 0.017453292519943295f)) {
        Serial.println("# ERROR: fallo calibracion/inicio de orientacion.");
        while (true) { delay(1000); }
    }
    Serial.println("# Calibracion OK");
}

void loop() {
    mpu6050_sample_t sample;
    if (!mpu6050_read(&sample)) {
        odometry_restart_tracking();
        Serial.println("# ERROR: lectura IMU invalida.");
        delay(SAMPLE_PERIOD_MS);
        return;
    }
    if (!odometry_update(sample.gyro_z_rads, millis())) {
        odometry_restart_tracking();
    }
    Serial.print(odometry_get_angle(), 4);
    Serial.println(",-1.0000");
    delay(SAMPLE_PERIOD_MS);
}
