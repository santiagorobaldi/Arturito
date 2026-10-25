#include "telemetry_app.h"
#include "hal_telemetry.h"

bool telemetry_app_init(void)
{
    return hal_telemetry_init();
}

void telemetry_app_update(void)
{
    hal_telemetry_update();
}

bool telemetry_app_send_line(const char *line)
{
    return hal_telemetry_send_line(line);
}
