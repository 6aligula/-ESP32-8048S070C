#include "screens.h"

void create_main_screen(lv_obj_t *scr) {
    // Fondo de la pantalla principal
    lv_obj_t *bg = lv_obj_create(scr);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xf0f0f0), LV_PART_MAIN); // Fondo gris claro
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, LV_PART_MAIN);

    // Logo
    lv_obj_t *logo = lv_label_create(scr);
    lv_label_set_text(logo, "LOGO");
    lv_obj_set_style_text_color(logo, lv_color_hex(0x0000FF), 0); // Azul
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_14, 0);
    lv_obj_align(logo, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado en la esquina superior izquierda

    // Título de la pantalla (LOG MCU)
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "LOG MCU");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0000FF), 0); // Azul
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10); // Centrado en la parte superior

    // Temperaturas
    lv_obj_t *label_temp1 = lv_label_create(scr);
    lv_label_set_text(label_temp1, "T1: 25°C");
    lv_obj_set_style_text_color(label_temp1, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_temp1, &lv_font_montserrat_14, 0);
    lv_obj_align(label_temp1, LV_ALIGN_TOP_LEFT, 50, 60); // Alineado a la izquierda

    lv_obj_t *label_temp2 = lv_label_create(scr);
    lv_label_set_text(label_temp2, "T2: 26°C");
    lv_obj_set_style_text_color(label_temp2, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_temp2, &lv_font_montserrat_14, 0);
    lv_obj_align(label_temp2, LV_ALIGN_TOP_LEFT, 50, 90); // Alineado debajo del primero

    // Volumen
    lv_obj_t *label_volume = lv_label_create(scr);
    lv_label_set_text(label_volume, "Volumen: XX ml");
    lv_obj_set_style_text_color(label_volume, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_volume, &lv_font_montserrat_14, 0);
    lv_obj_align(label_volume, LV_ALIGN_TOP_LEFT, 50, 120); // Alineado debajo del segundo

    // Contadores
    lv_obj_t *counters_box = lv_obj_create(scr);
    lv_obj_set_size(counters_box, 150, 100);
    lv_obj_set_style_bg_color(counters_box, lv_color_hex(0xDDA0DD), 0); // Color púrpura claro
    lv_obj_align(counters_box, LV_ALIGN_TOP_RIGHT, -10, 60); // Alineado a la derecha

    lv_obj_t *label_tot = lv_label_create(counters_box);
    lv_label_set_text(label_tot, "TOT: 35047");
    lv_obj_set_style_text_font(label_tot, &lv_font_montserrat_14, 0);
    lv_obj_align(label_tot, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado dentro del contenedor

    lv_obj_t *label_lot = lv_label_create(counters_box);
    lv_label_set_text(label_lot, "LOT: 2300");
    lv_obj_set_style_text_font(label_lot, &lv_font_montserrat_14, 0);
    lv_obj_align(label_lot, LV_ALIGN_TOP_LEFT, 10, 40); // Alineado debajo del primer contador

    // Botón Reset (movido fuera del contenedor y más abajo)
    lv_obj_t *btn_reset_counter = lv_btn_create(scr);
    lv_obj_set_size(btn_reset_counter, 100, 40); // Tamaño del botón
    lv_obj_align(btn_reset_counter, LV_ALIGN_TOP_RIGHT, -10, 170); // Alineado debajo de los contadores
    lv_obj_t *label_reset = lv_label_create(btn_reset_counter);
    lv_label_set_text(label_reset, "Reset");

    // Alarmas y errores
    lv_obj_t *alarm_box = lv_obj_create(scr);
    lv_obj_set_size(alarm_box, lv_pct(90), 100);
    lv_obj_set_style_bg_color(alarm_box, lv_color_hex(0xADD8E6), 0); // Azul claro
    lv_obj_align(alarm_box, LV_ALIGN_BOTTOM_MID, 0, -100); // Posicionado en la parte inferior

    lv_obj_t *label_alarm = lv_label_create(alarm_box);
    lv_label_set_text(label_alarm, "Alarmas / Errores:\n- Ninguna");
    lv_obj_set_style_text_font(label_alarm, &lv_font_montserrat_14, 0);
    lv_obj_align(label_alarm, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado dentro del cuadro de alarmas

    // Botones de control (Start, Stop, Reset)
    lv_obj_t *btn_start = lv_btn_create(scr);
    lv_obj_set_size(btn_start, 100, 50);
    lv_obj_set_style_bg_color(btn_start, lv_color_hex(0x32CD32), LV_PART_MAIN); // Verde
    lv_obj_align(btn_start, LV_ALIGN_BOTTOM_RIGHT, -10, -10); // Alineado en la esquina inferior derecha
    lv_obj_t *label_start = lv_label_create(btn_start);
    lv_label_set_text(label_start, "Start");

    lv_obj_t *btn_stop = lv_btn_create(scr);
    lv_obj_set_size(btn_stop, 100, 50);
    lv_obj_set_style_bg_color(btn_stop, lv_color_hex(0xFF4500), LV_PART_MAIN); // Rojo
    lv_obj_align(btn_stop, LV_ALIGN_BOTTOM_RIGHT, -120, -10); // Alineado a la izquierda del botón Start
    lv_obj_t *label_stop = lv_label_create(btn_stop);
    lv_label_set_text(label_stop, "Stop");

    lv_obj_t *btn_reset = lv_btn_create(scr);
    lv_obj_set_size(btn_reset, 100, 50);
    lv_obj_set_style_bg_color(btn_reset, lv_color_hex(0xFFA500), LV_PART_MAIN); // Naranja
    lv_obj_align(btn_reset, LV_ALIGN_BOTTOM_RIGHT, -230, -10); // Alineado a la izquierda del botón Stop
    lv_obj_t *label_reset_btn = lv_label_create(btn_reset);
    lv_label_set_text(label_reset_btn, "Reset");
}



void create_settings_screen(lv_obj_t *scr) {
    // Fondo de la pantalla de ajustes
    lv_obj_t *bg = lv_obj_create(scr);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_align(bg, LV_ALIGN_CENTER, 0, 0);

    // Etiqueta de ajustes
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Ajustes");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
