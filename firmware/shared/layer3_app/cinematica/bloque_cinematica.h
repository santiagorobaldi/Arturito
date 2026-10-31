#ifndef BLOQUE_CINEMATICA_H
#define BLOQUE_CINEMATICA_H

#include <stdbool.h>

const float R = 2.2f; // Radio de rueda en metros (44mm / 2) en cm
const float L = 5.0f; // Semieje / distancia al centro en cm

typedef struct {
    float x; // cm
    float y; // cm
    float theta; // radianes
} Posicion;

typedef struct { 
    float w_der; // rad/s
    float w_izq; // rad/s
} VelocidadesRuedas;

typedef struct {
    float v; // cm/s
    float w; // cm/s
} VelocidadesChasis;

typedef enum {
    MODO_PARADA = 0,
    MODO_TRANSITO,
    MODO_ESTACIONAMIENTO
} ModoControl;

// Función única que expone el bloque hacia afuera
VelocidadesRuedas cinematica_control_update(Posicion pose_actual, Posicion pose_meta, ModoControl modo);

VelocidadesChasis cinematica_directa(VelocidadesRuedas medido);

Posicion transformar_a_marco_global(Posicion local);

#endif