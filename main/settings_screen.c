#include "settings_screen.h"
#include "lvgl.h"
#include "uart_config.h" // Incluye la configuración de UART si es necesario
#include "esp_log.h"
#include "esp_lvgl_port.h"

// Funciones de callback para los botones de incremento, decremento y acciones
static void increment_callback(lv_event_t *e);
static void decrement_callback(lv_event_t *e);
static void apply_changes_callback(lv_event_t *e);
static void request_values_callback(lv_event_t *e);

void create_settings_screen(lv_obj_t *scr) {
    // Fondo de la pantalla
    lv_obj_t *bg = lv_obj_create(scr);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // Blanco
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, LV_PART_MAIN);

    // Fuente Montserrat 20
    static lv_style_t font_style;
    lv_style_init(&font_style);
    lv_style_set_text_font(&font_style, &lv_font_montserrat_20);

    // Título
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Menu de Ajustes");
    lv_obj_add_style(title, &font_style, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0); // Negro
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 50, 100); // Título inicia en 100 px

    // Parámetros
    const char *param_labels[] = {"Parametro 1", "Parametro 2", "Parametro 3", "Parametro 4"};
    int initial_values[] = {50, 100, 75, 25};

    // Margen superior inicial después del título
    int margin_top = 150; // El primer parámetro comienza debajo del título
    int row_spacing = 15; // Espaciado entre filas

    for (int i = 0; i < 4; i++) {
        // Crear contenedor para cada parámetro
        lv_obj_t *param_row = lv_obj_create(scr);
        lv_obj_set_size(param_row, lv_pct(90), 50);
        lv_obj_set_style_bg_color(param_row, lv_color_hex(0xF0F0F0), 0); // Gris claro
        lv_obj_align(param_row, LV_ALIGN_TOP_LEFT, 50, margin_top + i * (50 + row_spacing));

        // Eliminar scroll y padding del contenedor
        lv_obj_clear_flag(param_row, LV_OBJ_FLAG_SCROLLABLE); // Desactivar scroll
        lv_obj_set_style_pad_all(param_row, 0, LV_PART_MAIN); // Quitar padding
        lv_obj_set_style_clip_corner(param_row, true, LV_PART_MAIN); // Cortar contenido fuera de límites (opcional)

        // Etiqueta del parámetro
        lv_obj_t *label = lv_label_create(param_row);
        lv_label_set_text(label, param_labels[i]);
        lv_obj_add_style(label, &font_style, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);

        // Campo de valor
        lv_obj_t *value_label = lv_label_create(param_row);
        lv_label_set_text_fmt(value_label, "%d", initial_values[i]);
        lv_obj_add_style(value_label, &font_style, 0);
        lv_obj_align(value_label, LV_ALIGN_CENTER, 0, 0);

        // Botón para disminuir el valor
        lv_obj_t *btn_decrement = lv_btn_create(param_row);
        lv_obj_set_size(btn_decrement, 40, 40);
        lv_obj_set_style_bg_color(btn_decrement, lv_color_hex(0xFF0000), LV_PART_MAIN); // Rojo puro
        lv_obj_align(btn_decrement, LV_ALIGN_RIGHT_MID, -60, 0);
        lv_obj_t *label_decrement = lv_label_create(btn_decrement);
        lv_label_set_text(label_decrement, "-");
        lv_obj_add_style(label_decrement, &font_style, 0);
        lv_obj_center(label_decrement);
        lv_obj_add_event_cb(btn_decrement, decrement_callback, LV_EVENT_CLICKED, value_label);

        // Botón para aumentar el valor
        lv_obj_t *btn_increment = lv_btn_create(param_row);
        lv_obj_set_size(btn_increment, 40, 40);
        lv_obj_set_style_bg_color(btn_increment, lv_color_hex(0x00FF00), LV_PART_MAIN); // Verde puro
        lv_obj_align(btn_increment, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_t *label_increment = lv_label_create(btn_increment);
        lv_label_set_text(label_increment, "+");
        lv_obj_add_style(label_increment, &font_style, 0);
        lv_obj_center(label_increment);
        lv_obj_add_event_cb(btn_increment, increment_callback, LV_EVENT_CLICKED, value_label);
    }

    // Botón de "Pedir valores actuales"
    lv_obj_t *btn_request = lv_btn_create(scr);
    lv_obj_set_size(btn_request, 230, 50);
    lv_obj_set_style_bg_color(btn_request, lv_color_hex(0x888888), LV_PART_MAIN); // Gris oscuro
    lv_obj_align(btn_request, LV_ALIGN_BOTTOM_RIGHT, -20, -10); // Alineado en la parte inferior derecha
    lv_obj_t *label_request = lv_label_create(btn_request);
    lv_label_set_text(label_request, "Pedir valores actuales");
    lv_obj_add_style(label_request, &font_style, 0);
    lv_obj_center(label_request);
    lv_obj_add_event_cb(btn_request, request_values_callback, LV_EVENT_CLICKED, NULL);

    // Botón de "Aplicar Cambios"
    lv_obj_t *btn_apply = lv_btn_create(scr);
    lv_obj_set_size(btn_apply, 200, 50);
    lv_obj_set_style_bg_color(btn_apply, lv_color_hex(0x5B00FF), LV_PART_MAIN); // Morado
    // Alinear a la izquierda del botón `btn_request` sin solaparse
    lv_obj_align_to(btn_apply, btn_request, LV_ALIGN_OUT_LEFT_MID, -20, 0); // 20 píxeles de separación horizontal
    lv_obj_t *label_apply = lv_label_create(btn_apply);
    lv_label_set_text(label_apply, "Aplicar Cambios");
    lv_obj_add_style(label_apply, &font_style, 0);
    lv_obj_center(label_apply);
    lv_obj_add_event_cb(btn_apply, apply_changes_callback, LV_EVENT_CLICKED, NULL);

}

// Implementaciones de los callbacks
static void increment_callback(lv_event_t *e) {
    lv_obj_t *label = lv_event_get_user_data(e); // Obtener el campo de valor asociado
    const char *current_text = lv_label_get_text(label);
    int value = atoi(current_text); // Convertir texto a entero
    value += 1; // Incrementar valor
    lv_label_set_text_fmt(label, "%d", value); // Actualizar el texto del campo
    // Aquí puedes implementar el envío por UART si es necesario
}

static void decrement_callback(lv_event_t *e) {
    lv_obj_t *label = lv_event_get_user_data(e); // Obtener el campo de valor asociado
    const char *current_text = lv_label_get_text(label);
    int value = atoi(current_text); // Convertir texto a entero
    if (value > 0) value -= 1; // Decrementar valor (sin permitir negativos)
    lv_label_set_text_fmt(label, "%d", value); // Actualizar el texto del campo
    // Aquí puedes implementar el envío por UART si es necesario
}

static void apply_changes_callback(lv_event_t *e) {
    ESP_LOGI("Settings", "Apply Changes clicked");
    // Aquí puedes implementar la lógica para aplicar cambios y enviarlos por UART
}

static void request_values_callback(lv_event_t *e) {
    ESP_LOGI("Settings", "Request Current Values clicked");
    // Aquí puedes implementar la lógica para solicitar valores actuales por UART
}
