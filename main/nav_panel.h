// nav_panel.h
#ifndef NAV_PANEL_H
#define NAV_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef void (*nav_callback_t)(void);

typedef enum {
    LANG_ES,
    LANG_EN,
    LANG_FR,
    LANG_DE,
    LANG_COUNT
} Language;

typedef struct {
    lv_obj_t *nav_panel;
    lv_obj_t *logo;
    lv_obj_t *title;
    lv_obj_t *btn_home;
    lv_obj_t *btn_settings;
    lv_obj_t *btn_back;
    lv_obj_t *label_home;
    lv_obj_t *label_settings;
    lv_obj_t *label_back;
    Language current_language;
} NavPanel;

/**
 * @brief Crea un panel de navegación con botones.
 * @param parent El objeto padre (pantalla activa).
 * @param home_cb Callback para el botón de pantalla principal.
 * @param settings_cb Callback para el botón de ajustes.
 * @param back_cb Callback para el botón de atrás.
 * @return Un puntero a la estructura NavPanel creada.
 */
NavPanel* create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb);

#ifdef __cplusplus
}
#endif

#endif // NAV_PANEL_H
