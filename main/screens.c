#include "screens.h"
#include <string.h> // Para strlen
#include "driver/uart.h"
#include "uart_config.h"
#include "esp_log.h"


// Función genérica para enviar comandos UART
void send_command(const char *command) {
    uart_write_bytes(UART_PORT_NUM, command, strlen(command));
    ESP_LOGI("UART", "Enviado: %s", command);
}

// Callback genérico para manejar eventos de botones
void button_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // Obtiene el botón que disparó el evento
    const char *btn_label = lv_label_get_text(lv_obj_get_child(btn, 0)); // Obtiene el texto del botón

    if (strcmp(btn_label, "Start") == 0) {
        ESP_LOGI("BUTTON", "Botón Start presionado.");
        send_command("CMD:START\n");
    } else if (strcmp(btn_label, "Stop") == 0) {
        ESP_LOGI("BUTTON", "Botón Stop presionado.");
        send_command("CMD:STOP\n");
    } else if (strcmp(btn_label, "Reset") == 0) {
        ESP_LOGI("BUTTON", "Botón Reset presionado.");
        send_command("CMD:RESET\n");
    }
}

void create_main_screen(lv_obj_t *scr) {
    // Fondo de la pantalla principal
    lv_obj_t *bg = lv_obj_create(scr);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xf0f0f0), LV_PART_MAIN); // Fondo gris claro
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, LV_PART_MAIN);

    // Temperaturas
    lv_obj_t *label_temp1 = lv_label_create(scr);
    lv_label_set_text(label_temp1, "T1: 25°C");
    lv_obj_set_style_text_color(label_temp1, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_temp1, &lv_font_montserrat_14, 0);
    lv_obj_align(label_temp1, LV_ALIGN_TOP_LEFT, 50, 80); // Alineado a la izquierda

    lv_obj_t *label_temp2 = lv_label_create(scr);
    lv_label_set_text(label_temp2, "T2: 26°C");
    lv_obj_set_style_text_color(label_temp2, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_temp2, &lv_font_montserrat_14, 0);
    lv_obj_align(label_temp2, LV_ALIGN_TOP_LEFT, 50, 110); // Alineado debajo del primero

    // Volumen
    lv_obj_t *label_volume = lv_label_create(scr);
    lv_label_set_text(label_volume, "Volumen: XX ml");
    lv_obj_set_style_text_color(label_volume, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_volume, &lv_font_montserrat_14, 0);
    lv_obj_align(label_volume, LV_ALIGN_TOP_LEFT, 50, 140); // Alineado debajo del segundo

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

    // Crear botones y asignar el callback genérico
    struct {
        const char *label;
        lv_color_t color;
        lv_align_t align;
        int offset_x;
    } buttons[] = {
        {"Start", lv_color_hex(0x32CD32), LV_ALIGN_BOTTOM_RIGHT, -10},
        {"Stop", lv_color_hex(0xFF4500), LV_ALIGN_BOTTOM_RIGHT, -120},
        {"Reset", lv_color_hex(0xFFA500), LV_ALIGN_BOTTOM_RIGHT, -230},
    };

    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(scr);
        lv_obj_set_size(btn, 100, 50);
        lv_obj_set_style_bg_color(btn, buttons[i].color, LV_PART_MAIN);
        lv_obj_align(btn, buttons[i].align, buttons[i].offset_x, -10);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, buttons[i].label);
        lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, NULL);
    }

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
