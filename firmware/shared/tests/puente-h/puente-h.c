#include "traction.h"
#include "hal_time.h"

#include <stdint.h>
#include <stdbool.h>

/*
Código de prototipo utilizando las implementaciones
A REVISAR:
- Actualmente la velocidad se considera PWM (entre 0 y 100)
- Actualmente la velocidad es única variable para ambas ruedas (PID necesitara distinción)

*/

int main (void){
    if (!traction_init()) return 1;
    
    if (!traction_set_velocity(50)) return 1;
    
    while (true){
        if (!traction_start(TRACTION_FORWARD)) return 1;
        hal_delay(3000);
        
        if (!traction_stop()) return 1;
        hal_delay(3000);
        
        if (!traction_start(TRACTION_BACKWARD)) return 1;
        hal_delay(3000);
        
        if (!traction_stop()) return 1;
        hal_delay(3000);
    
        if (!traction_turn(false)) return 1;
        hal_delay(3000);
    
        if (!traction_stop()) return 1;
        hal_delay(3000);
    
        if (!traction_turn(true)) return 1;
        hal_delay(3000);
    
        if (!traction_stop()) return 1;
        hal_delay(3000);
    }
    
    return 0;
}
