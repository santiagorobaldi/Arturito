#ifndef TEST_TOF_H
#define TEST_TOF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializa y configura el test del VL53L0X. */
void test_tof_init(void);

/* Ejecuta una adquisicion de distancia cuando corresponde. */
void test_tof_update(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_TOF_H */
