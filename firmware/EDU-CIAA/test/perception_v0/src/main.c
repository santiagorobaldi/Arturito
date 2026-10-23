#include "sapi.h"
#include "perception_v0.h"
#include "hal_telemetry.h"
#include "hal_i2c.h"

#include "FreeRTOS.h"
#include "task.h"

/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void) {
  boardConfig();
  if (!hal_i2c_init(HAL_I2C_BUS_A)) {
    hal_telemetry_write("# ERROR: no se pudo inicializar el bus I2C A.");
    while (TRUE) {
    }
  }
  if (!perception_v0_start()) {
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
