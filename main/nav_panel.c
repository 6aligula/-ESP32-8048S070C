#include "nav_panel.h"

lv_obj_t *create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb) {
    // Crear el panel de navegación
    lv_obj_t *nav_panel = lv_obj_create(parent);
    lv_obj_set_size(nav_panel, lv_pct(100), 70);  // Ancho 100%, alto 70px
    lv_obj_align(nav_panel, LV_ALIGN_TOP_MID, 0, 0);

    // Estilo del panel
    lv_obj_set_style_bg_color(nav_panel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(nav_panel, LV_OPA_COVER, LV_PART_MAIN);

    // Agregar el logo al panel
    lv_obj_t *logo = lv_label_create(nav_panel);
    lv_label_set_text(logo, "LOGO");
    lv_obj_set_style_text_color(logo, lv_color_hex(0xFFFFFF), 0); // Blanco
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_14, 0);
    lv_obj_align(logo, LV_ALIGN_LEFT_MID, 10, 0); // Alineado a la izquierda

    // Agregar el título (LOG MCU) al panel
    lv_obj_t *title = lv_label_create(nav_panel);
    lv_label_set_text(title, "MCV");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0); // Blanco
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 70, 0); // Alineado cerca del logo

    // Botón de inicio
    lv_obj_t *btn_home = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_home, 80, 30);
    lv_obj_align(btn_home, LV_ALIGN_CENTER, -120, 0); // Alineado cerca del centro
    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Inicio");
    lv_obj_add_event_cb(btn_home, (lv_event_cb_t)home_cb, LV_EVENT_CLICKED, NULL);

    // Botón de ajustes
    lv_obj_t *btn_settings = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_settings, 80, 30);
    lv_obj_align(btn_settings, LV_ALIGN_CENTER, 20, 0); // Alineado cerca del centro
    lv_obj_t *label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, "Ajustes");
    lv_obj_align(label_settings, LV_ALIGN_CENTER, -3, 0); // Mover texto ligeramente
    lv_obj_add_event_cb(btn_settings, (lv_event_cb_t)settings_cb, LV_EVENT_CLICKED, NULL);

    // Botón de atrás
    lv_obj_t *btn_back = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_back, 80, 30);
    lv_obj_align(btn_back, LV_ALIGN_RIGHT_MID, -10, 0); // Alineado a la derecha
    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Atras");
    lv_obj_add_event_cb(btn_back, (lv_event_cb_t)back_cb, LV_EVENT_CLICKED, NULL);

    return nav_panel;
}
