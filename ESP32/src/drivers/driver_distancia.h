#ifndef DRIVER_DISTANCIA_H
#define DRIVER_DISTANCIA_H

// Capa 2: capa 1 ya entrega cm crudos. Acá solo se filtra lo que no sirve
// (fuera del rango útil del sensor), igual que hacía tu sketch de
// Processing al descartar valores fuera de 5-150cm.

void distancia_init();

// -1 si no hay lectura válida o está fuera de rango.
float distancia_get_cm();

#endif // DRIVER_DISTANCIA_H
