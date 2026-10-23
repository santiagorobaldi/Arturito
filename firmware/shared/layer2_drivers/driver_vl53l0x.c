#include "driver_vl53l0x.h"
#include "hal_vl53l0x.h"

typedef struct {
    float measured_cm;
    float real_cm;
} CalibrationPoint;

/* High Speed, calibracion sobre carton.
 * Cada punto relaciona distancia cruda del sensor con distancia real.
 */
static const CalibrationPoint calibration[] = {
    /* Punto estimado prolongando la recta entre las mediciones de 5 y 10 cm. */
    { 3.064f,  2.0f },
    { 6.61f,  5.0f },
    {12.51f, 10.0f },
    {17.75f, 15.0f },
    {22.63f, 20.0f },
    {27.38f, 25.0f },
    {31.84f, 30.0f },
    {35.63f, 35.0f },
    {39.92f, 40.0f },
    {43.10f, 45.0f },
    {45.79f, 50.0f },
    {48.10f, 55.0f },
    {50.50f, 60.0f },
    {53.86f, 65.0f }
};

#define CAL_COUNT (sizeof(calibration) / sizeof(calibration[0]))

static float interpolate_cm(
    float measured_cm,
    const CalibrationPoint *a,
    const CalibrationPoint *b
) {
    return a->real_cm +
        (measured_cm - a->measured_cm) *
        (b->real_cm - a->real_cm) /
        (b->measured_cm - a->measured_cm);
}

bool vl53l0x_init(void) {
    return hal_vl53l0x_init();
}

float vl53l0x_validate_cm(float distancia_cm) {
#if defined(BOARD_ESP32)
    /* En ESP32, validar la entrada cruda contra los extremos de la tabla. */
    if (!(distancia_cm >= calibration[0].measured_cm &&
          distancia_cm <= calibration[CAL_COUNT - 1U].measured_cm)) {
#else
    /* EDU-CIAA conserva su rango generico: no usa esta calibracion ESP32. */
    if (!(distancia_cm >= 0.1f && distancia_cm <= 150.0f)) {
#endif
        return -1.0f;
    }

    return distancia_cm;
}

float vl53l0x_get_raw_distance_cm(void) {
    return hal_vl53l0x_get_distance_cm();
}

float vl53l0x_calibrate_cm(float distancia_cruda_cm) {
    if (vl53l0x_validate_cm(distancia_cruda_cm) < 0.0f) {
        return -1.0f;
    }

    for (unsigned int i = 0U; i < CAL_COUNT - 1U; ++i) {
        if (distancia_cruda_cm <= calibration[i + 1U].measured_cm) {
            return interpolate_cm(
                distancia_cruda_cm,
                &calibration[i],
                &calibration[i + 1U]
            );
        }
    }

    return -1.0f;
}

float vl53l0x_get_distance_cm(void) {
    const float distancia_cruda_cm = vl53l0x_get_raw_distance_cm();
#if defined(BOARD_ESP32)
    /* Esta tabla se midio con el VL53L0X en High Speed sobre ESP32. */
    return vl53l0x_calibrate_cm(distancia_cruda_cm);
#else
    /* No aplicar en EDU-CIAA una tabla obtenida con otro HAL/perfil. */
    return vl53l0x_validate_cm(distancia_cruda_cm);
#endif
}
