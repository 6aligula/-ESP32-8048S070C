#include "nav_panel.h"
#include "logo.h" // Archivo generado con la imagen (debe estar definido como LVGL compatible)

lv_obj_t *create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb) {
    // Crear el panel de navegación
    lv_obj_t *nav_panel = lv_obj_create(parent);
    lv_obj_set_size(nav_panel, lv_pct(100), 70);  // Ancho 100%, alto 70px
    lv_obj_align(nav_panel, LV_ALIGN_TOP_MID, 0, 0);

    // Estilo del panel
    lv_obj_set_style_bg_color(nav_panel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(nav_panel, LV_OPA_COVER, LV_PART_MAIN);

    // Agregar el logo como imagen al panel
    lv_obj_t *logo_img = lv_img_create(nav_panel); // Cambiado el nombre del objeto para evitar confusión
    lv_img_set_src(logo_img, &logo); // Usamos el descriptor `logo` del archivo generado
    lv_obj_align(logo_img, LV_ALIGN_LEFT_MID, 10, 0); // Alineado a la izquierda

    // Escalar la imagen si es más grande de lo necesario
    lv_img_set_zoom(logo_img, 128); // Escala al 50% del tamaño original


    // Crear un estilo para el título
    static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_20); // Aplicar la fuente de 20 píxeles

    // Crear el título y aplicar el estilo
    lv_obj_t *title = lv_label_create(nav_panel);
    lv_label_set_text(title, "MCV");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 90, 0);

    // Crear un estilo para los textos de los botones
    static lv_style_t style_button_label;
    lv_style_init(&style_button_label);
    lv_style_set_text_font(&style_button_label, &lv_font_montserrat_20); // Fuente Montserrat 20 píxeles

    // Botón de inicio
    lv_obj_t *btn_home = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_home, 110, 50);
    lv_obj_align(btn_home, LV_ALIGN_CENTER, -120, 0); // Alineado cerca del centro
    lv_obj_t *label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, "Inicio");
    lv_obj_add_style(label_home, &style_button_label, 0); // Aplicar el estilo
    lv_obj_center(label_home); // Centrar la etiqueta dentro del botón
    lv_obj_add_event_cb(btn_home, (lv_event_cb_t)home_cb, LV_EVENT_CLICKED, NULL);

    // Botón de ajustes
    lv_obj_t *btn_settings = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_settings, 110, 50);
    lv_obj_align(btn_settings, LV_ALIGN_CENTER, 20, 0); // Alineado cerca del centro
    lv_obj_t *label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, "Ajustes");
    lv_obj_add_style(label_settings, &style_button_label, 0); // Aplicar el estilo
    // lv_obj_align(label_settings, LV_ALIGN_CENTER, -3, 0); // Mover texto ligeramente
    lv_obj_add_event_cb(btn_settings, (lv_event_cb_t)settings_cb, LV_EVENT_CLICKED, NULL);

    // Botón de atrás
    lv_obj_t *btn_back = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_back, 110, 50);
    lv_obj_align(btn_back, LV_ALIGN_RIGHT_MID, -10, 0); // Alineado a la derecha
    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Atras");
    lv_obj_add_style(label_back, &style_button_label, 0); // Aplicar el estilo
    lv_obj_add_event_cb(btn_back, (lv_event_cb_t)back_cb, LV_EVENT_CLICKED, NULL);

    return nav_panel;
}
