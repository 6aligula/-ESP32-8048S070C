#ifndef SCREENS_H
#define SCREENS_H

#include "lvgl.h"

/* Inicializa la pantalla principal */
void create_main_screen(lv_obj_t *scr);

// Declaración de la función en screens.h
void uart_receive_task(void *arg);

#endif // SCREENS_H
