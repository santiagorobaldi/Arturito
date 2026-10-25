#include "main.h"
#include "telemetry_app.h"
#include "hal_telemetry.h"
#include "hal_time.h"

static bool ready = false;

void app_init(void)
{
    hal_telemetry_write("TEST TELEMETRIA: recibiendo datos cada 1 s.");
    ready = telemetry_app_init();
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
