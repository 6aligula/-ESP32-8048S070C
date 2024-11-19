#include "nav_panel.h"

lv_obj_t *create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb) {
    lv_obj_t *nav_panel = lv_obj_create(parent);
    lv_obj_set_size(nav_panel, lv_pct(100), 50);  // Ancho 100%, alto 50px
    lv_obj_align(nav_panel, LV_ALIGN_TOP_MID, 0, 0);

    // Estilo del panel (opcional)
    lv_obj_set_style_bg_color(nav_panel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(nav_panel, LV_OPA_COVER, LV_PART_MAIN);

    // Botón de inicio
    lv_obj_t *btn_home = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_home, 70, 30);
    lv_obj_align(btn_home, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Inicio");
    lv_obj_add_event_cb(btn_home, (lv_event_cb_t)home_cb, LV_EVENT_CLICKED, NULL);

    // Botón de ajustes
    lv_obj_t *btn_settings = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_settings, 70, 30);
    lv_obj_align(btn_settings, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, "Ajustes");
    lv_obj_add_event_cb(btn_settings, (lv_event_cb_t)settings_cb, LV_EVENT_CLICKED, NULL);

    // Botón de atrás
    lv_obj_t *btn_back = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_back, 70, 30);
    lv_obj_align(btn_back, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Atras");
    lv_obj_add_event_cb(btn_back, (lv_event_cb_t)back_cb, LV_EVENT_CLICKED, NULL);

    return nav_panel;
}
