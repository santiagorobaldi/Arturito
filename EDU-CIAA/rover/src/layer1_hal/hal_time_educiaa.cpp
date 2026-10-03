#include "hal_time.h"
#include "sapi.h"

void hal_delay(uint32_t ms) {
    delay(ms);
}

uint32_t hal_millis(void) {
    return tickRead();
}
