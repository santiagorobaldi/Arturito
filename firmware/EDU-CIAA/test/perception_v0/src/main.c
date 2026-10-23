#include "sapi.h"
#include "perception_v0.h"
#include "hal_telemetry.h"

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void) {
  boardConfig();
  if (!perception_v0_init()) {
    hal_telemetry_write("# ERROR: no se pudo inicializar la aplicacion.");
    while (TRUE) {
    }
  }

  while (TRUE) {
    perception_v0_step();
  }
}
