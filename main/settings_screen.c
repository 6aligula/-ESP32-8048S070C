#include "settings_screen.h"
#include "lvgl.h"
#include "uart_config.h" // Incluye la configuración de UART si es necesario
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include <stdlib.h> // Para malloc y free
#include <string.h>
#include "uart_utils.h" // Incluye las funciones de UART centralizadas

// Definición de la estructura btn_data_t
typedef struct {
    lv_obj_t *label;    // Etiqueta del valor asociado
    int increment;      // Incremento (positivo o negativo)
    lv_timer_t *timer;  // Temporizador para manejar mantenimientos prolongados
} btn_data_t;

// Funciones de callback para los botones de incremento, decremento y acciones
static void increment_callback(lv_event_t *e);
static void decrement_callback(lv_event_t *e);
static void apply_changes_callback(lv_event_t *e);
static void request_values_callback(lv_event_t *e);
static void button_press_callback(lv_event_t *e);
static void button_release_callback(lv_event_t *e);
static void btn_destroy_callback(lv_event_t *e);

static void settings_data_handler(const char *data) {
    // Procesa los datos específicos de la pantalla de ajustes
    if (strncmp(data, "SETTINGS", 8) == 0) {
        ESP_LOGI("SETTINGS", "Datos de configuración recibidos: %s", data);
    }
}

// Función para manejar incrementos o decrementos
static void update_value(btn_data_t *btn_data) {
    if (btn_data == NULL || btn_data->label == NULL) {
        ESP_LOGE("Settings", "btn_data or label is NULL in update_value");
        return;
    }

    const char *current_text = lv_label_get_text(btn_data->label);
    int value = atoi(current_text);
    value += btn_data->increment;

    // Opcional: Validaciones de valores (por ejemplo, mantener entre 0 y 100)
    // if (value < 0) value = 0;
    // if (value > 100) value = 100;

    lv_label_set_text_fmt(btn_data->label, "%d", value);

    // Aquí puedes implementar el envío por UART si es necesario
    // uart_send_value(btn_data->label, value);
}

// Función estática para manejar el temporizador
static void timer_callback(lv_timer_t *timer) {
    // Utiliza la función de LVGL para obtener el user_data
    btn_data_t *btn_data = (btn_data_t *)lv_timer_get_user_data(timer);
    if (btn_data != NULL) {
        update_value(btn_data);  // Actualizar el valor en pasos de 10 o -10
    } else {
        ESP_LOGE("Settings", "btn_data is NULL en timer_callback");
    }
}

// Callback para botones cuando se mantiene pulsado
static void button_press_callback(lv_event_t *e) {
    btn_data_t *btn_data = (btn_data_t *)lv_event_get_user_data(e);

    if (btn_data == NULL) {
        ESP_LOGE("Settings", "btn_data is NULL en button_press_callback");
        return;
    }

    // Crear un temporizador para incrementar/decrementar mientras se mantenga pulsado
    btn_data->timer = lv_timer_create(timer_callback, 200, btn_data);

    // Configurar el incremento inicial grande
    btn_data->increment *= 10; // Incrementos grandes (10 en 10 o -10 en -10)
}

// Callback para soltar el botón
static void button_release_callback(lv_event_t *e) {
    btn_data_t *btn_data = (btn_data_t *)lv_event_get_user_data(e);

    if (btn_data == NULL) {
        ESP_LOGE("Settings", "btn_data is NULL en button_release_callback");
        return;
    }

    // Detener el temporizador
    if (btn_data->timer) {
        lv_timer_del(btn_data->timer);
        btn_data->timer = NULL;
    }

    // Restaurar el incremento normal
    btn_data->increment /= 10; // Volver a incrementos pequeños (1 en 1 o -1 en -1)
}

// Incrementar el valor en un clic
static void increment_callback(lv_event_t *e) {
    btn_data_t *btn_data = (btn_data_t *)lv_event_get_user_data(e);
    if (btn_data != NULL) {
        update_value(btn_data);  // Incrementar en pasos de 1
    } else {
        ESP_LOGE("Settings", "btn_data is NULL en increment_callback");
    }
}

// Decrementar el valor en un clic
static void decrement_callback(lv_event_t *e) {
    btn_data_t *btn_data = (btn_data_t *)lv_event_get_user_data(e);
    if (btn_data != NULL) {
        update_value(btn_data);  // Decrementar en pasos de 1
    } else {
        ESP_LOGE("Settings", "btn_data is NULL en decrement_callback");
    }
}

// Callback para liberar la memoria cuando se destruye el botón
static void btn_destroy_callback(lv_event_t *e) {
    btn_data_t *btn_data = (btn_data_t *)lv_event_get_user_data(e);
    if (btn_data != NULL) {
        // Asegurarse de que el temporizador esté detenido
        if (btn_data->timer) {
            lv_timer_del(btn_data->timer);
            btn_data->timer = NULL;
        }
        // Liberar la memoria
        free(btn_data);
    }
}

void create_settings_screen(lv_obj_t *scr) {
    ESP_LOGI("SETTINGS", "Creando pantalla de ajustes");

    // Configura el handler para la pantalla de ajustes
    uart_register_handler(settings_data_handler);
    
    // Fondo de la pantalla
    lv_obj_t *bg = lv_obj_create(scr);
    lv_obj_set_size(bg, LV_PCT(100), LV_PCT(100));
    // lv_obj_add_style(bg, &lv_style_plain_color, LV_PART_MAIN); // Eliminar esta línea
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

    // Contenedor desplazable para parámetros
    lv_obj_t *param_scroll = lv_obj_create(scr);
    lv_obj_set_size(param_scroll, LV_PCT(100), LV_VER_RES - 200); // Tamaño ajustado
    lv_obj_align(param_scroll, LV_ALIGN_TOP_MID, 0, 150);         // Debajo del título
    lv_obj_set_scroll_dir(param_scroll, LV_DIR_VER);             // Scroll solo vertical
    lv_obj_set_style_bg_opa(param_scroll, LV_OPA_TRANSP, 0);     // Fondo transparente
    //lv_obj_set_scroll_snap_y(param_scroll, LV_SCROLL_SNAP_START); // Snap ajustado al inicio
    lv_obj_set_scroll_snap_y(param_scroll, LV_SCROLL_SNAP_NONE); // Sin Snap
    lv_obj_set_style_pad_top(param_scroll, 0, LV_PART_MAIN);     // Sin padding superior
    lv_obj_set_style_pad_bottom(param_scroll, 0, LV_PART_MAIN);  // Sin padding inferior

    // Añadir el Checkbox llamado "check" alineado a la derecha en la misma fila que el título
    lv_obj_t *checkbox = lv_checkbox_create(scr);
    lv_checkbox_set_text(checkbox, "check");
    lv_obj_add_style(checkbox, &font_style, 0);
    lv_obj_align(checkbox, LV_ALIGN_TOP_RIGHT, -50, 100); // Ajusta los offsets según sea necesario

    // Parámetros
    const char *param_labels[] = {"Parametro 1", "Parametro 2", "Parametro 3", "Parametro 4", "Parametro 5", "Parametro 6", "Parametro 7", "Parametro 8"};
    int initial_values[] = {50, 100, 75, 25, 33, 77, 34, 32};
    const int num_params = sizeof(initial_values) / sizeof(initial_values[0]);

    // Margen superior inicial después del título
    int row_spacing = 15;  // Espaciado entre filas

    for (int i = 0; i < num_params; i++) {
        // Crear contenedor para cada parámetro
        lv_obj_t *param_row = lv_obj_create(param_scroll);
        lv_obj_set_size(param_row, LV_PCT(90), 50);
        lv_obj_set_style_bg_color(param_row, lv_color_hex(0xF0F0F0), LV_PART_MAIN); // Gris claro
        lv_obj_set_style_bg_opa(param_row, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(param_row, LV_ALIGN_TOP_MID, 0, i * (50 + row_spacing));
        // Eliminar padding interno de cada fila
        lv_obj_set_style_pad_all(param_row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_top(param_row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(param_row, 0, LV_PART_MAIN);

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

        // Crear estructura de datos para el botón de incremento
        btn_data_t *btn_inc_data = malloc(sizeof(btn_data_t));
        if (btn_inc_data == NULL) {
            ESP_LOGE("Settings", "Failed to allocate memory for increment button data");
            continue; // O manejar el error según sea necesario
        }
        btn_inc_data->label = value_label;
        btn_inc_data->increment = 1;
        btn_inc_data->timer = NULL;

        // Crear botón para aumentar el valor
        lv_obj_t *btn_increment = lv_btn_create(param_row);
        lv_obj_set_size(btn_increment, 40, 40);
        // lv_obj_add_style(btn_increment, &lv_style_plain_color, LV_PART_MAIN); // Eliminar esta línea
        lv_obj_set_style_bg_color(btn_increment, lv_color_hex(0x00FF00), LV_PART_MAIN); // Verde puro
        lv_obj_set_style_bg_opa(btn_increment, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(btn_increment, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_t *label_increment = lv_label_create(btn_increment);
        lv_label_set_text(label_increment, "+");
        lv_obj_add_style(label_increment, &font_style, 0);
        lv_obj_center(label_increment);
        lv_obj_add_event_cb(btn_increment, increment_callback, LV_EVENT_CLICKED, btn_inc_data);
        lv_obj_add_event_cb(btn_increment, button_press_callback, LV_EVENT_PRESSED, btn_inc_data);
        lv_obj_add_event_cb(btn_increment, button_release_callback, LV_EVENT_RELEASED, btn_inc_data);
        lv_obj_add_event_cb(btn_increment, btn_destroy_callback, LV_EVENT_DELETE, btn_inc_data);

        // Crear estructura de datos para el botón de decremento
        btn_data_t *btn_dec_data = malloc(sizeof(btn_data_t));
        if (btn_dec_data == NULL) {
            ESP_LOGE("Settings", "Failed to allocate memory for decrement button data");
            // Liberar btn_inc_data si no puedes asignar btn_dec_data
            free(btn_inc_data);
            continue; // O manejar el error según sea necesario
        }
        btn_dec_data->label = value_label;
        btn_dec_data->increment = -1;
        btn_dec_data->timer = NULL;

        // Crear botón para disminuir el valor
        lv_obj_t *btn_decrement = lv_btn_create(param_row);
        lv_obj_set_size(btn_decrement, 40, 40);
        // lv_obj_add_style(btn_decrement, &lv_style_plain_color, LV_PART_MAIN); // Eliminar esta línea
        lv_obj_set_style_bg_color(btn_decrement, lv_color_hex(0xFF0000), LV_PART_MAIN); // Rojo puro
        lv_obj_set_style_bg_opa(btn_decrement, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(btn_decrement, LV_ALIGN_RIGHT_MID, -60, 0);
        lv_obj_t *label_decrement = lv_label_create(btn_decrement);
        lv_label_set_text(label_decrement, "-");
        lv_obj_add_style(label_decrement, &font_style, 0);
        lv_obj_center(label_decrement);
        lv_obj_add_event_cb(btn_decrement, decrement_callback, LV_EVENT_CLICKED, btn_dec_data);
        lv_obj_add_event_cb(btn_decrement, button_press_callback, LV_EVENT_PRESSED, btn_dec_data);
        lv_obj_add_event_cb(btn_decrement, button_release_callback, LV_EVENT_RELEASED, btn_dec_data);
        lv_obj_add_event_cb(btn_decrement, btn_destroy_callback, LV_EVENT_DELETE, btn_dec_data);
    }

    // Botón de "Pedir valores actuales"
    lv_obj_t *btn_request = lv_btn_create(scr);
    lv_obj_set_size(btn_request, 230, 50);
    // lv_obj_add_style(btn_request, &lv_style_plain_color, LV_PART_MAIN); // Eliminar esta línea
    lv_obj_set_style_bg_color(btn_request, lv_color_hex(0x888888), LV_PART_MAIN); // Gris oscuro
    lv_obj_set_style_bg_opa(btn_request, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(btn_request, LV_ALIGN_BOTTOM_RIGHT, -20, -10); // Alineado en la parte inferior derecha
    lv_obj_t *label_request = lv_label_create(btn_request);
    lv_label_set_text(label_request, "Pedir valores actuales");
    lv_obj_add_style(label_request, &font_style, 0);
    lv_obj_center(label_request);
    lv_obj_add_event_cb(btn_request, request_values_callback, LV_EVENT_CLICKED, NULL);

    // Botón de "Aplicar Cambios"
    lv_obj_t *btn_apply = lv_btn_create(scr);
    lv_obj_set_size(btn_apply, 200, 50);
    // lv_obj_add_style(btn_apply, &lv_style_plain_color, LV_PART_MAIN); // Eliminar esta línea
    lv_obj_set_style_bg_color(btn_apply, lv_color_hex(0x5B00FF), LV_PART_MAIN); // Morado
    lv_obj_set_style_bg_opa(btn_apply, LV_OPA_COVER, LV_PART_MAIN);
    // Alinear a la izquierda del botón `btn_request` sin solaparse
    lv_obj_align_to(btn_apply, btn_request, LV_ALIGN_OUT_LEFT_MID, -20, 0); // 20 píxeles de separación horizontal
    lv_obj_t *label_apply = lv_label_create(btn_apply);
    lv_label_set_text(label_apply, "Aplicar Cambios");
    lv_obj_add_style(label_apply, &font_style, 0);
    lv_obj_center(label_apply);
    lv_obj_add_event_cb(btn_apply, apply_changes_callback, LV_EVENT_CLICKED, NULL);
}

// Callback para aplicar cambios
static void apply_changes_callback(lv_event_t *e) {
    ESP_LOGI("Settings", "Apply Changes clicked");
    send_command("SPA*"); // Enviar comando por UART usando función centralizada
}

// Callback para solicitar valores actuales
static void request_values_callback(lv_event_t *e) {
    ESP_LOGI("Settings", "Request Current Values clicked");
    // Aquí puedes implementar la lógica para solicitar valores actuales por UART
    send_command("SPV*"); // Enviar comando por UART usando función centralizada
}
