#include "main.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include "hal_time.h"

static bool ready = false;

void app_init(void)
{
    ready = telemetry_app_init();
    if (!ready) {
        hal_telemetry_write("ERROR: no se pudo iniciar la telemetria.");
        return;
    }
    telemetry_app_send_line("TEST TELEMETRIA: recibiendo datos cada 1 s.");
}

void app_update(void)
{
    if (!ready) {
        return;
    }
    telemetry_app_update();
    telemetry_app_send_line("recibiendo datos");
    hal_delay(1000U);
}
