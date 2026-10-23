#include "hal_time.h"
#include "sapi.h"

void hal_delay(uint32_t ms) {
    delay(ms);
}

uint32_t hal_millis(void) {
    return tickRead();
}

uint32_t hal_micros(void) {
    /* sAPI expone tickRead en milisegundos; esta conversion conserva el ABI,
     * aunque la resolucion real de EDU-CIAA sigue siendo de 1 ms. */
    return tickRead() * 1000U;
}
