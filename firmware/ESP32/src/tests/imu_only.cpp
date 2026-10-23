#include "driver_mpu6050.h"
#include "driver_mpu6050_config.h"

#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {
constexpr uint32_t kSamplePeriodUs = 10000U;
constexpr uint32_t kMaxSampleGapUs = 100000U;
constexpr double kRadiansToDegrees = 57.29577951308232;
constexpr uint32_t kReportPeriodUs = 200000U;

struct Experiment {
    bool active = false;
    bool has_previous = false;
    double angle_rad = 0.0;
    double previous_rate_rads = 0.0;
    float latest_rate_rads = 0.0f;
    uint32_t previous_time_us = 0U;
    uint32_t start_time_us = 0U;
    uint32_t last_report_us = 0U;
    uint32_t valid_samples = 0U;
    uint32_t invalid_samples = 0U;
    uint32_t long_gaps = 0U;
    uint32_t late_periods = 0U;
};

Experiment experiment;
uint32_t next_sample_us = 0U;
char command_buffer[40];
size_t command_length = 0U;

void reset_experiment() {
    experiment.angle_rad = 0.0;
    experiment.previous_rate_rads = 0.0;
    experiment.latest_rate_rads = 0.0f;
    experiment.has_previous = false;
    experiment.valid_samples = 0U;
    experiment.invalid_samples = 0U;
    experiment.long_gaps = 0U;
    experiment.late_periods = 0U;
}

void start_experiment() {
    reset_experiment();
    experiment.start_time_us = micros();
    experiment.last_report_us = experiment.start_time_us;
    experiment.active = true;
    Serial.println("# EXPERIMENTO INICIADO: empieza el movimiento o espera en reposo.");
}

void stop_experiment(double expected_degrees) {
    if (!experiment.active) {
        Serial.println("# ERROR: no hay experimento activo; envia start primero.");
        return;
    }

    experiment.active = false;
    const uint32_t elapsed_us = micros() - experiment.start_time_us;
    const double elapsed_s = (double)elapsed_us / 1000000.0;
    const double measured_degrees = experiment.angle_rad * kRadiansToDegrees;
    const double error_degrees = measured_degrees - expected_degrees;
    const double mean_rate_dps = elapsed_s > 0.0 ? measured_degrees / elapsed_s : 0.0;
    const double sample_rate_hz = elapsed_s > 0.0 ?
        (double)experiment.valid_samples / elapsed_s : 0.0;

    Serial.printf(
        "# RESULT esperado_deg=%.2f integrado_deg=%.3f error_deg=%.3f tiempo_s=%.3f velocidad_media_dps=%.2f frecuencia_hz=%.1f muestras=%lu invalidas=%lu periodos_tarde=%lu pausas_largas=%lu\n",
        expected_degrees, measured_degrees, error_degrees, elapsed_s,
        mean_rate_dps, sample_rate_hz, (unsigned long)experiment.valid_samples,
        (unsigned long)experiment.invalid_samples,
        (unsigned long)experiment.late_periods,
        (unsigned long)experiment.long_gaps);
}

void calibrate_while_stationary() {
    if (experiment.active) {
        Serial.println("# ERROR: detene el experimento antes de calibrar.");
        return;
    }

    Serial.println("# CALIBRACION: no muevas Arturito durante unos 5 segundos.");
    if (!mpu6050_calibrate_gyro_z(MPU6050_CALIBRATION_SAMPLE_COUNT,
                                  MPU6050_CALIBRATION_INTERVAL_MS)) {
        Serial.println("# ERROR: fallo la calibracion del giroscopio.");
        return;
    }
    reset_experiment();
    Serial.println("# CALIBRACION OK.");
}

void print_help() {
    Serial.println("# Comandos:");
    Serial.println("#   cal          recalibra bias; robot quieto");
    Serial.println("#   start        pone theta=0 e inicia medicion");
    Serial.println("#   stop <deg>   termina y compara contra angulo real con signo");
    Serial.println("#   stop 0       sirve para medir deriva en reposo");
    Serial.println("# Convencion: theta positivo segun regla de mano derecha del eje Z.");
}

void process_command(const char *command) {
    if (strcmp(command, "start") == 0) {
        start_experiment();
        return;
    }
    if (strcmp(command, "cal") == 0) {
        calibrate_while_stationary();
        return;
    }

    double expected_degrees;
    char trailing;
    if (sscanf(command, "stop %lf %c", &expected_degrees, &trailing) == 1 &&
        isfinite(expected_degrees)) {
        stop_experiment(expected_degrees);
        return;
    }

    if (strcmp(command, "help") == 0) {
        print_help();
    } else {
        Serial.println("# Comando no reconocido; envia help.");
    }
}

void poll_serial() {
    while (Serial.available() > 0) {
        const char ch = (char)Serial.read();
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            command_buffer[command_length] = '\0';
            if (command_length > 0U) {
                process_command(command_buffer);
            }
            command_length = 0U;
        } else if (command_length + 1U < sizeof(command_buffer)) {
            command_buffer[command_length++] = ch;
        } else {
            command_length = 0U;
            Serial.println("# ERROR: comando demasiado largo.");
        }
    }
}

void take_sample() {
    float gyro_z_rads;
    if (!mpu6050_read_gyro_z(&gyro_z_rads) || !isfinite(gyro_z_rads)) {
        experiment.has_previous = false;
        if (experiment.active) {
            ++experiment.invalid_samples;
        }
        return;
    }

    const uint32_t sample_time_us = micros();
    experiment.latest_rate_rads = gyro_z_rads;
    if (experiment.active) {
        ++experiment.valid_samples;
    }

    if (experiment.has_previous) {
        const uint32_t elapsed_us = sample_time_us - experiment.previous_time_us;
        if (elapsed_us == 0U) {
            experiment.has_previous = false;
            if (experiment.active) {
                ++experiment.invalid_samples;
            }
        } else if (elapsed_us > kMaxSampleGapUs) {
            experiment.has_previous = false;
            if (experiment.active) {
                ++experiment.long_gaps;
            }
        } else if (experiment.active) {
            const double dt_s = (double)elapsed_us / 1000000.0;
            const double mean_rate =
                (experiment.previous_rate_rads + gyro_z_rads) * 0.5;
            experiment.angle_rad += mean_rate * dt_s;
        }
    }

    experiment.previous_rate_rads = gyro_z_rads;
    experiment.previous_time_us = sample_time_us;
    experiment.has_previous = true;

    if (experiment.active &&
        (uint32_t)(sample_time_us - experiment.last_report_us) >= kReportPeriodUs) {
        const double elapsed_s =
            (double)(sample_time_us - experiment.start_time_us) / 1000000.0;
        const double gyro_z_dps =
            (double)experiment.latest_rate_rads * kRadiansToDegrees;
        Serial.printf("# SAMPLE t_s=%.3f gyro_z_dps=%.2f theta_deg=%.3f\n",
                      elapsed_s, gyro_z_dps,
                      experiment.angle_rad * kRadiansToDegrees);
        experiment.last_report_us = sample_time_us;
    }
}
}

void setup() {
    Serial.begin(115200);
    Serial.println("# test imu_only: calibracion angular MPU6050");

    if (!mpu6050_init(MPU6050_I2C_ADDRESS)) {
        Serial.println("# ERROR: IMU no encontrada. Halt.");
        while (true) {
            delay(1000U);
        }
    }

    Serial.println("# Config ESP32: gyro +/-250 dps, DLPF 44 Hz, ODR 100 Hz.");
    Serial.println("# Calibrando bias al iniciar; mantene Arturito quieto.");
    if (!mpu6050_calibrate_gyro_z(MPU6050_CALIBRATION_SAMPLE_COUNT,
                                  MPU6050_CALIBRATION_INTERVAL_MS)) {
        Serial.println("# ERROR: fallo la calibracion del giroscopio.");
        while (true) {
            delay(1000U);
        }
    }

    reset_experiment();
    next_sample_us = micros();
    Serial.println("# CALIBRACION OK. Envia help para ver comandos.");
    print_help();
}

void loop() {
    poll_serial();

    const uint32_t now_us = micros();
    if ((int32_t)(now_us - next_sample_us) >= 0) {
        const uint32_t lateness_us = now_us - next_sample_us;
        if (lateness_us >= kSamplePeriodUs) {
            experiment.late_periods += lateness_us / kSamplePeriodUs;
            next_sample_us = now_us + kSamplePeriodUs;
        } else {
            next_sample_us += kSamplePeriodUs;
        }
        take_sample();
    }

    delay(1U);
}
