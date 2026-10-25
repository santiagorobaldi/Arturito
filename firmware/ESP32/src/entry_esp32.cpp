
#include <Arduino.h>

#include "main.h"

/* Inicializa los recursos de la plataforma y la aplicación. */
void setup()
{
    Serial.begin(115200);
    app_init();
}

/* Ejecuta continuamente la aplicación compartida. */
void loop()
{
    app_update();
}
