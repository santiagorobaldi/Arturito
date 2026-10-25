
#ifndef APP_MAIN_H
#define APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Inicializa la aplicación seleccionada. */
void app_init(void);

/* Ejecuta una iteración de la aplicación seleccionada. */
void app_update(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
