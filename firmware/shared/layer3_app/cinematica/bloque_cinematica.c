#include "bloque_cinematica.h"
#include <math.h>

// ----- SUBMÓDULOS INTERNOS (Privados) -----

// Paso 1: Transformación de Marco Global a Local 
static Posicion transformar_a_marco_local(Posicion actual, Posicion meta) {
    float dx_g = meta.x - actual.x;
    float dy_g = meta.y - actual.y;
    float dtheta_g = meta.theta - actual.theta;

    Posicion local;
    local.x =  cosf(actual.theta) * dx_g + sinf(actual.theta) * dy_g;
    local.y = -sinf(actual.theta) * dx_g + cosf(actual.theta) * dy_g;
    local.theta = dtheta_g;
    return local;
}

// Paso 2: Control C(s) según el modo
static void control_polar(Posicion e, float *v_out, float *w_out){
    float rho = sqrtf(e.x * e.x + e.y * e.y);
    float alpha = atan2f(e.y, e.x);
    
    // Se asegura de que de la vuelta más corta
    if(e.theta>M_PI) e.theta -= 2.0F*M_PI;
    if(e.theta<=-M_PI) e.theta += 2.0F*M_PI;

    float beta  = e.theta - alpha;
    
    // Constantes sintonizadas del controlador
    float K_RHO = 0.5f, K_ALPHA = 1.2f, K_BETA = -0.3f;
    
    // Cuando me acerco a la meta desactivo K_ALPHA porque atan2() es muy sensible 
    if(rho<5)K_ALPHA = 0; 
    
    *v_out = K_RHO * rho;
    *w_out = K_ALPHA * alpha + K_BETA * beta;
}

static void control_parada(Posicion e,float *v_out, float *w_out){
    *v_out = 0;
    *w_out = 0;
}

static void control(Posicion error_local, ModoControl modo, float *v_out, float *w_out) {
    switch (modo) {
        case MODO_PARADA:
            control_parada(error_local,v_out,w_out);
            break;

        case MODO_ESTACIONAMIENTO: { // Control Polar Siegwart
            control_polar(error_local,v_out,w_out);
            break;
        }

        case MODO_TRANSITO:
            // Algoritmo de seguimiento de trayectoria (ej. Pure Pursuit)
            break;
    }
}

// Paso 3: Cinemática Inversa [v, w] -> [w_der, w_izq] 
static VelocidadesRuedas cinematica_inversa(float v, float w) {
    VelocidadesRuedas consigna_ruedas;
    consigna_ruedas.w_der = (v + L * w) / R;
    consigna_ruedas.w_izq = (v - L * w) / R;
    return consigna_ruedas;
}


// ----- INTERFAZ -----

// - FUNCIÓN PRINCIPAL (Totalmente Secuencial) -
VelocidadesRuedas cinematica_control_update(Posicion pose_actual, Posicion pose_meta, ModoControl modo) {
    // 1. Transformar diferencias globales a error local
    Posicion error_local = transformar_a_marco_local(pose_actual, pose_meta);

    // 2. Calcular velocidades de chasis [v, w]
    float v = 0.0f, w = 0.0f;
    control(error_local, modo, &v, &w);

    // 3. Convertir a velocidades objetivo individuales de rueda
    VelocidadesRuedas consigna_ruedas = cinematica_inversa(v, w);

    // 4. Retornar las consignas para el Bloque de Tracción
    return consigna_ruedas;
}

// - FUNCIÓN UTILIZADA POR ESTIMACIÓN DE POSICIÓN -
// convierte un vector de desplazamiento visto desde el marco local del robot
// a un vector de desplazamiento visto desde el marco global.
Posicion transformar_a_marco_global(Posicion local){
    Posicion desplazamiento;
    
    desplazamiento.x = cosf(local.theta) * local.x - sinf(local.theta) * local.y;
    desplazamiento.y = sinf(local.theta) * local.x + cosf(local.theta) * local.y;
    desplazamiento.theta = local.theta;

    return desplazamiento;
}

// - FUNCIÓN UTILIZADA POR ESTIMACIÓN DE POSICIÓN -
// permite saber en base a la velocidad de las ruedas la velocidad lineal
// y angular del chasis.
VelocidadesChasis cinematica_directa(VelocidadesRuedas medido){
    VelocidadesChasis v_chasis_medido;

    v_chasis_medido.v = (R/2)*medido.w_der + (R/2)*medido.w_izq;
    v_chasis_medido.w = R*(medido.w_der - medido.w_izq)/(2*L);

    return v_chasis_medido;
}
