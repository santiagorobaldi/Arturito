#ifndef CONFIG_H
#define CONFIG_H

// En este proyecto (dentro de firmware_v3) la placa siempre es EDU-CIAA;
// no hace falta el define condicional que usamos en el proyecto de ESP32.
#define BOARD_EDUCIAA

#define SAMPLE_PERIOD_MS  30   // ~30Hz, igual que en la versión de ESP32

#endif // CONFIG_H
