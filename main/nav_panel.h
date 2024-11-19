#ifndef NAV_PANEL_H
#define NAV_PANEL_H

// #include "../managed_components/lvgl__lvgl/lvgl.h"

#include "lvgl.h"

typedef void (*nav_callback_t)(void);

/**
 * @brief Crea un panel de navegación con botones.
 * @param parent El objeto padre (pantalla activa).
 * @param home_cb Callback para el botón de pantalla principal.
 * @param settings_cb Callback para el botón de ajustes.
 * @param back_cb Callback para el botón de atrás.
 */
lv_obj_t *create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb);

#endif // NAV_PANEL_H
