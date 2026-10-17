#ifndef TRACTION_H
#define TRACTION_H

#include <stdint.h>
#include <stdbool.h>

typedef enum{
    TRACTION_FORWARD,
    TRACTION_BACKWARD
}traction_dir_t;

/*
Velocity = v
Direction = dir
*/



bool traction_init(void);
bool traction_set_velocity(int v);
bool traction_start(traction_dir_t dir);
bool traction_stop(void);
bool traction_turn(bool right);