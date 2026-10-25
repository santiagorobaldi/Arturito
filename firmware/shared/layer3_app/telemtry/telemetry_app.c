#include "telemetry_app.h"
#include "telemetry_driver.h"

void telemetry_app_init(void) {
    telemetry_driver_init();
}

void telemetry_app_update(void) {
    telemetry_driver_poll();
}

void telemetry_log_info(const char *msg) {
    telemetry_send_text(TELEMETRY_LOG_INFO, msg);
}

void telemetry_plot_value(const char *name, float val) {
    telemetry_send_variable_float(name, val);
}