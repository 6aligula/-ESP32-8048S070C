#include "screens.h"
#include <string.h> // Para strlen
#include "driver/uart.h"
#include "uart_config.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "uart_utils.h"

// Definiciones de errores
#define NUM_ERRORES 8

const char *mensajes_errores[NUM_ERRORES] = {
    "Error 0: Sobrecalentamiento T1",
    "Error 1: Sobrecalentamiento T2",
    "Error 2: Volumen fuera de rango",
    "Error 3: Comunicacion UART fallida",
    "Error 4: Sensor de temperatura desconectado",
    "Error 5: Bateria baja",
    "Error 6: Memoria insuficiente",
    "Error 7: Otro error desconocido"
};

// Función para verificar si un bit específico está activado
bool is_bit_set(uint8_t byte, uint8_t bit_position) {
    if (bit_position >= 8) {
        // Posición de bit inválida
        return false;
    }
    return (byte & (1 << bit_position)) != 0;
}

// Función para obtener los mensajes de errores activos
char* get_active_errors(uint8_t error_byte) {
    // Estimar el tamaño máximo necesario
    size_t max_size = NUM_ERRORES * 50; // Ajusta según tus mensajes
    char *error_messages = (char*)malloc(max_size);
    if (error_messages == NULL) {
        // Manejar error de asignación de memoria
        return NULL;
    }
    error_messages[0] = '\0'; // Inicializar cadena vacía

    for (uint8_t i = 0; i < NUM_ERRORES; i++) {
        if (is_bit_set(error_byte, i)) {
            strcat(error_messages, mensajes_errores[i]);
            strcat(error_messages, "\n"); // Nueva línea para cada error
        }
    }

    if (strlen(error_messages) == 0) {
        strcpy(error_messages, "Ninguna");
    }

    return error_messages;
}

static lv_obj_t *label_temp1;
static lv_obj_t *label_temp2;
static lv_obj_t *label_volume;
static lv_obj_t *label_alarm; // Etiqueta para errores

// Definir la estructura para los datos UART
typedef struct {
    float t1;
    float t2;
    int vol;
    uint8_t errores;
} uart_data_t;

static uart_data_t latest_data;

// Función de callback para actualizar las etiquetas
static void update_labels_callback(void *param) {
    uart_data_t *data = (uart_data_t *)param;
    lv_label_set_text_fmt(label_temp1, "T1: %.2f °C", data->t1);
    lv_label_set_text_fmt(label_temp2, "T2: %.2f °C", data->t2);
    lv_label_set_text_fmt(label_volume, "Volumen: %d ml", data->vol);
    
    // Obtener los mensajes de errores activos
    char *errores_activos = get_active_errors(data->errores);
    if (errores_activos != NULL) {
        lv_label_set_text(label_alarm, errores_activos);
        free(errores_activos); // Liberar la memoria asignada
    } else {
        lv_label_set_text(label_alarm, "Error al procesar errores");
    }
}

static void screen_data_handler(const char *data) {
    ESP_LOGI("SCREEN", "Handler invocado con: %s", data);
    if (strncmp(data, "DATA", 4) == 0) {
        float t1, t2;
        int vol;
        uint8_t errores = 0;
        // Parsear ERR=0xXX
        int parsed = sscanf(data, "DATA:T1=%f;T2=%f;VOL=%d;ERR=0x%hhX;", &t1, &t2, &vol, &errores);
        if (parsed >= 3) {
            ESP_LOGI("SCREEN", "Datos procesados: T1=%.2f, T2=%.2f, Volumen=%d", t1, t2, vol);

            // Almacenar los datos más recientes
            latest_data.t1 = t1;
            latest_data.t2 = t2;
            latest_data.vol = vol;
            latest_data.errores = errores;

            // Programar la actualización de las etiquetas en el loop principal de LVGL
            lv_async_call(update_labels_callback, &latest_data);
        } else {
            ESP_LOGW("SCREEN", "Formato de datos incorrecto: %s", data);
        }
    }
}

// Callback genérico para manejar eventos de botones
void button_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // Obtiene el botón que disparó el evento
    const char *btn_label = lv_label_get_text(lv_obj_get_child(btn, 0)); // Obtiene el texto del botón

    if (strcmp(btn_label, "START") == 0) {
        ESP_LOGI("BUTTON", "Botón Start presionado.");
        send_command("CMD:STA01*");
    } else if (strcmp(btn_label, "STOP") == 0) {
        ESP_LOGI("BUTTON", "Botón Stop presionado.");
        send_command("CMD:STO01*");
    } else if (strcmp(btn_label, "RESET") == 0) {
        ESP_LOGI("BUTTON", "Botón Reset presionado.");
        send_command("CMD:RES01*");
    }else if (strcmp(btn_label, "RST CNT") == 0) {
        ESP_LOGI("BUTTON", "BTN Reset Counters pressed.");
        send_command("CMD:RSC01*");
    }
}

void create_main_screen(lv_obj_t *scr) {
    ESP_LOGI("SCREEN", "Creando pantalla principal");

    // Configura el handler para la pantalla principal
    uart_register_handler(screen_data_handler);
    // Fondo de la pantalla principal
    lv_obj_t *bg = lv_obj_create(scr);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0xf0f0f0), LV_PART_MAIN); // Fondo gris claro
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, LV_PART_MAIN);

    // Crear un estilo para los textos de los botones
    static lv_style_t style_button_label;
    lv_style_init(&style_button_label);
    lv_style_set_text_font(&style_button_label, &lv_font_montserrat_20); // Fuente Montserrat 20 píxeles

    // Temperaturas
    label_temp1 = lv_label_create(scr);
    lv_label_set_text(label_temp1, "T1: -- °C");
    lv_obj_set_style_text_color(label_temp1, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_temp1, &lv_font_montserrat_20, 0);
    lv_obj_align(label_temp1, LV_ALIGN_TOP_LEFT, 50, 100); // Alineado a la izquierda

    label_temp2 = lv_label_create(scr);
    lv_label_set_text(label_temp2, "T2: -- °C");
    lv_obj_set_style_text_color(label_temp2, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_temp2, &lv_font_montserrat_20, 0);
    lv_obj_align(label_temp2, LV_ALIGN_TOP_LEFT, 50, 130); // Alineado debajo del primero

    // Volumen
    label_volume = lv_label_create(scr);
    lv_label_set_text(label_volume, "Volumen: -- ml");
    lv_obj_set_style_text_color(label_volume, lv_color_hex(0xFFA500), 0); // Naranja
    lv_obj_set_style_text_font(label_volume, &lv_font_montserrat_20, 0);
    lv_obj_align(label_volume, LV_ALIGN_TOP_LEFT, 50, 160); // Alineado debajo del segundo

    // Contadores
    lv_obj_t *counters_box = lv_obj_create(scr);
    lv_obj_set_size(counters_box, 170, 120);
    lv_obj_set_style_bg_color(counters_box, lv_color_hex(0xDDA0DD), 0); // Color púrpura claro
    lv_obj_align(counters_box, LV_ALIGN_TOP_RIGHT, -10, 90); // Alineado a la derecha

    lv_obj_t *label_tot = lv_label_create(counters_box);
    lv_label_set_text(label_tot, "TOT: 35047");
    lv_obj_set_style_text_font(label_tot, &lv_font_montserrat_20, 0);
    lv_obj_align(label_tot, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado dentro del contenedor

    lv_obj_t *label_lot = lv_label_create(counters_box);
    lv_label_set_text(label_lot, "LOT: 2300");
    lv_obj_set_style_text_font(label_lot, &lv_font_montserrat_20, 0);
    lv_obj_align(label_lot, LV_ALIGN_TOP_LEFT, 10, 40); // Alineado debajo del primer contador

    // Botón Reset (movido fuera del contenedor y más abajo)
    lv_obj_t *btn_reset_counter = lv_btn_create(scr);
    lv_obj_set_size(btn_reset_counter, 100, 40); // Tamaño del botón
    lv_obj_align(btn_reset_counter, LV_ALIGN_TOP_RIGHT, -10, 210); // Alineado debajo de los contadores
    lv_obj_t *label_reset = lv_label_create(btn_reset_counter);
    lv_label_set_text(label_reset, "RST CNT");
    lv_obj_add_style(label_reset, &style_button_label, 0); // Aplicar el estilo
    lv_obj_center(label_reset); // Centrar la etiqueta dentro del botón

    // Alarmas y errores
    lv_obj_t *alarm_box = lv_obj_create(scr);
    lv_obj_set_size(alarm_box, lv_pct(90), 100);
    lv_obj_set_style_bg_color(alarm_box, lv_color_hex(0xADD8E6), 0); // Azul claro
    lv_obj_align(alarm_box, LV_ALIGN_BOTTOM_MID, 0, -100); // Posicionado en la parte inferior
    lv_obj_set_style_pad_top(alarm_box, 0, LV_PART_MAIN); // Sin padding superior

    label_alarm = lv_label_create(alarm_box);
    lv_label_set_text(label_alarm, "Alarmas / Errores:\n- Ninguna");
    lv_obj_set_style_text_font(label_alarm, &lv_font_montserrat_20, 0);
    lv_obj_align(label_alarm, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado dentro del cuadro de alarmas

    // Crear botones y asignar el callback genérico
    struct {
        const char *label;
        lv_color_t color;
        lv_align_t align;
        int offset_x;
    } buttons[] = {
        {"START", lv_color_hex(0x32CD32), LV_ALIGN_BOTTOM_RIGHT, -10},
        {"STOP", lv_color_hex(0xFF4500), LV_ALIGN_BOTTOM_RIGHT, -150},
        {"RESET", lv_color_hex(0xFFA500), LV_ALIGN_BOTTOM_RIGHT, -300},
    };

    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(scr);
        lv_obj_set_size(btn, 120, 50);
        lv_obj_set_style_bg_color(btn, buttons[i].color, LV_PART_MAIN);
        lv_obj_align(btn, buttons[i].align, buttons[i].offset_x, -10);
        // Crear la etiqueta para el botón
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, buttons[i].label);
        lv_obj_add_style(label, &style_button_label, 0); // Aplicar el estilo al texto del botón
        lv_obj_center(label); // Centrar la etiqueta dentro del botón
        lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, NULL);
    }
    //create_spinbox();

}
