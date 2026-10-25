#include "main.h"
#include "test_hal_time.h"

/* Adapta la prueba del reloj al ciclo de vida comun de cada plataforma. */
void app_init(void)
{
    test_hal_time_init();
}

void app_update(void)
{
    test_hal_time_update();
}
