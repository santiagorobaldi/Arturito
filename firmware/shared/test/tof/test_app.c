#include "main.h"
#include "test_tof.h"

/* Adapta el test ToF al ciclo de vida comun de cada plataforma. */
void app_init(void)
{
    test_tof_init();
}

void app_update(void)
{
    test_tof_update();
}
