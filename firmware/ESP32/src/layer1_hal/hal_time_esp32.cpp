#include "hal_time.h"

#include <Arduino.h>

void hal_delay(uint32_t ms) {
    delay(ms);
}

uint32_t hal_millis(void) {
    return millis();
}
