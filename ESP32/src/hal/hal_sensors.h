#ifndef HAL_SENSORS_H
#define HAL_SENSORS_H

// Capa 1: cada placa implementa esto como pueda (I2C crudo, librería del
// fabricante, driver de sAPI, lo que haga falta). Capa 2 para arriba no
// sabe ni le importa cómo se consiguió el dato, solo que viene en estas
// unidades fijas:
//   - distancia en centímetros
//   - velocidad angular en radianes/segundo
// Esto es adrede distinto de "capa 1 = solo pines/I2C": para estos dos
// sensores puntuales, las librerías de cada plataforma ya envuelven el I2C
// ellas mismas, así que forzar un HAL de I2C genérico debajo era trabajo
// de más sin beneficio real. Si más adelante agregan un sensor sin
// librería propia en ninguna plataforma, ahí sí conviene bajar esta
// función a un hal_i2c genérico auxiliar.

void hal_sensors_init();

// Distancia leída por el sensor TOF, en cm. -1 si la lectura no es válida.
float hal_get_distance_cm();

// Velocidad angular en Z del giroscopio, en rad/s (sin filtrar, sin offset).
float hal_get_gyro_z_rads();

#endif // HAL_SENSORS_H
