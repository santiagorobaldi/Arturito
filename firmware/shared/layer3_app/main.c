
#include "main.h"

#include "test_hal_time.h"

/* Inicializa el test seleccionado. */
void app_init(void)
{
    test_hal_time_init();
}

/* Ejecuta el test seleccionado. */
void app_update(void)
{
    test_hal_time_update();
}
