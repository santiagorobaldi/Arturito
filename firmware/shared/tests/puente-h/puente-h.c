#include "traction.h"
#include "hal_time.h"

#include <stdint.h>
#include <stdbool.h>


int main (void){
    if (!traction_init()) return 1;
    
    while (true){
        if (!traction_start_a_b(0.1,0.1)) return 1;
        hal_delay(3000);
        
        if (!traction_stop()) return 1;
        hal_delay(3000);
        
        if (!traction_start_a_b(-0.1,-0.1)) return 1;
        hal_delay(3000);
        
        if (!traction_stop()) return 1;
        hal_delay(3000);

        if (!traction_start_a_b(-0.1,0.1)) return 1;
        hal_delay(3000);
        
        if (!traction_stop()) return 1;
        hal_delay(3000);

        if (!traction_start_a_b(0.1,-0.1)) return 1;
        hal_delay(3000);
        
        if (!traction_stop()) return 1;
        hal_delay(3000);

    }
    
    return 0;
}
