#include "sapi.h"
#include "app_runtime.h"
#include "hal_telemetry.h"

#include "FreeRTOS.h"
#include "task.h"

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void) {
  boardConfig();
  if (!app_runtime_start()) {
    hal_telemetry_write("# ERROR: no se pudo crear la tarea de inicio.");
    while (TRUE) {
    }
  }

  vTaskStartScheduler();
  hal_telemetry_write("# ERROR: FreeRTOS no pudo iniciar el scheduler.");
  while (TRUE) {
  }

  return 0;
}
