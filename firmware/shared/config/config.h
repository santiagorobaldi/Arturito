#ifndef CONFIG_H
#define CONFIG_H

/* BOARD_ESP32 / BOARD_EDUCIAA las define el build, no este header. */

#define SAMPLE_PERIOD_MS  30
#define IMU_I2C_ADDR      0x68

#define DIST_MIN_M  0.05f
#define DIST_MAX_M  1.50f

#endif /* CONFIG_H */
