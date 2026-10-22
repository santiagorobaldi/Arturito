#ifndef TRACTION_H
#define TRACTION_H
#endif

#include <stdint.h>
#include <stdbool.h>

bool traction_init(void);
bool traction_set_ref(int vel_a, int vel_b);
bool traction_tick();
bool traction_stop(void);