#include "odometry.h"
#include <math.h>

static bool initialized, has_previous;
static float angle, deadband, previous_velocity;
static uint32_t previous_time;

bool odometry_init(float initial_angle_rad, float deadband_rads) {
    if (deadband_rads < 0.0f) {
        return false;
    }
    angle = initial_angle_rad;
    deadband = deadband_rads;
    has_previous = false;
    initialized = true;
    return true;
}

bool odometry_update(float angular_velocity_rads, uint32_t sample_time_ms) {
    if (!initialized) {
        return false;
    }
    float velocity = fabsf(angular_velocity_rads) <= deadband ?
                     0.0f : angular_velocity_rads;
    if (has_previous) {
        uint32_t elapsed = sample_time_ms - previous_time;
        if (elapsed == 0U || elapsed >= 0x80000000U) {
            return false;
        }
        double dt_s = (double)elapsed / 1000.0;
        double average_velocity = ((double)previous_velocity + velocity) / 2.0;
        angle += (float)(average_velocity * dt_s);
    }
    previous_velocity = velocity;
    previous_time = sample_time_ms;
    has_previous = true;
    return true;
}

float odometry_get_angle(void) { return angle; }

bool odometry_reset(float angle_rad) {
    if (!initialized) {
        return false;
    }
    angle = angle_rad;
    has_previous = false;
    return true;
}

bool odometry_restart_tracking(void) {
    return odometry_reset(odometry_get_angle());
}
