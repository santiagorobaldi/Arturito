#include "distance.h"
#include "hal_vl53l0x.h"

#include <Arduino.h>
#include <math.h>
#include <stdio.h>

namespace {
constexpr uint16_t kSampleCount = 100U;

struct RunningStats {
    double mean = 0.0;
    double sum_squared_deltas = 0.0;
    uint16_t count = 0U;
};

void add_sample(RunningStats &stats, float value) {
    ++stats.count;
    const double delta = (double)value - stats.mean;
    stats.mean += delta / (double)stats.count;
    stats.sum_squared_deltas += delta * ((double)value - stats.mean);
}

void print_stats(const char *label, const char *deviation_label,
                 const RunningStats &stats) {
    char line[144];
    if (stats.count > 0U) {
        const double stddev = sqrt(stats.sum_squared_deltas / (double)stats.count);
        snprintf(line, sizeof(line),
                 "# Promedio %s: %.2f cm, desviacion estandar %s: %.2f cm (%u/%u validas)",
                 label, stats.mean, deviation_label, stddev,
                 (unsigned int)stats.count, (unsigned int)kSampleCount);
    } else {
        snprintf(line, sizeof(line), "# Promedio %s: sin datos (0/%u validas)",
                 label, (unsigned int)kSampleCount);
    }
    Serial.println(line);
}

void run_measurement_batch() {
    RunningStats raw_stats;
    RunningStats calibrated_stats;
    hal_vl53l0x_reset_diagnostics();

    for (uint16_t i = 0U; i < kSampleCount; ++i) {
        const float raw_cm = distance_get_raw();
        if (isfinite(raw_cm) && raw_cm > 0.0f) {
            add_sample(raw_stats, raw_cm);

            const float calibrated_cm = distance_calibrate(raw_cm);
            if (isfinite(calibrated_cm) && calibrated_cm > 0.0f) {
                add_sample(calibrated_stats, calibrated_cm);
            }
        }
    }

    print_stats("crudo", "cruda", raw_stats);
    print_stats("calibrado", "calibrada", calibrated_stats);

    hal_vl53l0x_diagnostics_t diagnostics;
    hal_vl53l0x_get_diagnostics(&diagnostics);
    char line[160];
    snprintf(line, sizeof(line),
             "# ToF diagnostico: API=%u, status[0..5]=%u,%u,%u,%u,%u,%u, otros=%u, 0xFFFF=%u",
             (unsigned int)diagnostics.api_errors,
             (unsigned int)diagnostics.range_status_counts[0],
             (unsigned int)diagnostics.range_status_counts[1],
             (unsigned int)diagnostics.range_status_counts[2],
             (unsigned int)diagnostics.range_status_counts[3],
             (unsigned int)diagnostics.range_status_counts[4],
             (unsigned int)diagnostics.range_status_counts[5],
             (unsigned int)diagnostics.other_status_count,
             (unsigned int)diagnostics.sentinel_count);
    Serial.println(line);
}
}

void setup() {
    Serial.begin(115200);
    Serial.println("# test vl53l0x_only: promedio crudo y calibrado");

    if (!distance_init()) {
        Serial.println("# ERROR: VL53L0X no encontrado. Halt.");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("# ToF OK");
    Serial.println("# Escribe ok y Enter para promediar 100 lecturas.");
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command.equalsIgnoreCase("ok")) {
            run_measurement_batch();
        } else if (command.length() > 0U) {
            Serial.println("# Comando no reconocido; escribe ok.");
        }
    }

    delay(10U);
}
