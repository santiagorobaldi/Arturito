
#ifndef TEST_HAL_TIME_H
#define TEST_HAL_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializa los recursos necesarios para la prueba. */
void test_hal_time_init(void);

/* Ejecuta las comprobaciones periódicas del reloj. */
void test_hal_time_update(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_HAL_TIME_H */
