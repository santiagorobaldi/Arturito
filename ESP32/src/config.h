#ifndef CONFIG_H
#define CONFIG_H

// ---- Selección de placa (capa 1 se implementa distinto según esto) ----
// Definir UNA sola de estas dos, ya sea acá o como build flag del compilador.
#define BOARD_ESP32
// #define BOARD_EDUCIAA

#if defined(BOARD_ESP32) && defined(BOARD_EDUCIAA)
  #error "Definí solo una placa a la vez"
#endif
#if !defined(BOARD_ESP32) && !defined(BOARD_EDUCIAA)
  #error "Definí BOARD_ESP32 o BOARD_EDUCIAA"
#endif

// ---- Parámetros compartidos (no dependen de la placa) ----
#define IMU_I2C_ADDR      0x68
#define SAMPLE_PERIOD_MS  30   // ~30Hz, igual que tu loop original

#endif // CONFIG_H
