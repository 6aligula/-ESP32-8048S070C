#include "screens.h"
#include <string.h> // Para strlen
#include "driver/uart.h"
#include "uart_config.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

static lv_obj_t *label_temp1;
static lv_obj_t *label_temp2;
static lv_obj_t *label_volume;

// editor numeric
void create_spinbox(void) {
    // Crear el spinbox
    lv_obj_t * spinbox = lv_spinbox_create(lv_scr_act());

    // Configurar el rang del spinbox (mínim i màxim)
    lv_spinbox_set_range(spinbox, 0, 100);  // Valor entre 0 i 100

    // Configurar el valor inicial
    lv_spinbox_set_value(spinbox, 50);  // Valor inicial 50

    // Configurar l'increment (quantitat de canvi per pas)
    lv_spinbox_set_step(spinbox, 1);  // Increment de 1 unitat per pas

    // Centrar el spinbox a la pantalla
    lv_obj_align(spinbox, LV_ALIGN_CENTER, 0, 0);

    // Configurar el nombre de dígits i decimals
    lv_spinbox_set_digit_format(spinbox, 3, 0);  // 3 dígits i 0 decimals

    // Personalitzar l'estil (color de text)
    lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_color(&style, lv_color_hex(0xFF0000));  // Text de color vermell
    lv_obj_add_style(spinbox, &style, LV_PART_MAIN);
}

// Tarea para recibir datos del UART
void uart_receive_task(void *arg) {
    char rx_buffer[128];
    const TickType_t delay_ticks = pdMS_TO_TICKS(100); // 5 segundos en ticks
    while (true) {
        // Leer datos del UART
        int length = uart_read_bytes(UART_PORT_NUM, (uint8_t *)rx_buffer, sizeof(rx_buffer) - 1, pdMS_TO_TICKS(1000));
        if (length > 0) {
            rx_buffer[length] = '\0'; // Asegurar terminación de cadena
            float t1, t2;
            int vol;
            char RCVdata[100];



            // Parsear la trama recibida
            ESP_LOGI("UART", ">>RCV>>%s\n", rx_buffer);

            if (strncmp(rx_buffer, "PARAM", 5) == 0)
            {
                //ESP_LOGI("UART", "PARAM: %s\n", rx_buffer); 
                ESP_LOGI("UART", "PARAM\n"); 
            }

            if (strncmp(rx_buffer, "DATA", 4) == 0)
            {
                //ESP_LOGI("UART", "DATA: %s\n", rx_buffer);
                ESP_LOGI("UART", "DATA\n"); 
                // decode
                if (sscanf(rx_buffer, "DATA:T1=%f;T2=%f;VOL=%d;", &t1, &t2, &vol) == 3)
                {
                    ESP_LOGI("UART", "-->DECODED: - T1: %.2f, T2: %.2f, Vol: %d", t1, t2, vol);

                    // Actualizar etiquetas en la pantalla (debe ser en el contexto de LVGL)
                    lvgl_port_lock(0); // Bloquear para actualizar desde otra tarea
                    lv_label_set_text_fmt(label_temp1, "T1: %.2f °C", t1);
                    lv_label_set_text_fmt(label_temp2, "T2: %.2f °C", t2);
                    lv_label_set_text_fmt(label_volume, "Volumen: %d ml", vol);
                    lvgl_port_unlock();
                }
                else
                {
                    ESP_LOGW("UART", "Trama no DATA válida: %s", rx_buffer);
                }

            }
        }
        vTaskDelay(delay_ticks);
    }
}


// Función genérica para enviar comandos UART
void send_command(const char *command) {
    uart_write_bytes(UART_PORT_NUM, command, strlen(command));
    ESP_LOGI("UART", "Enviado: %s", command);
}

// Callback genérico para manejar eventos de botones
void button_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e); // Obtiene el botón que disparó el evento
    const char *btn_label = lv_label_get_text(lv_obj_get_child(btn, 0)); // Obtiene el texto del botón

    if (strcmp(btn_label, "START") == 0) {
        ESP_LOGI("BUTTON", "Botón Start presionado.");
        send_command("STA01*");
    } else if (strcmp(btn_label, "STOP") == 0) {
        ESP_LOGI("BUTTON", "Botón Stop presionado.");
        send_command("STO01*");
    } else if (strcmp(btn_label, "RESET") == 0) {
        ESP_LOGI("BUTTON", "Botón Reset presionado.");
        send_command("RES01*");
    }else if (strcmp(btn_label, "RST CNT") == 0) {
        ESP_LOGI("BUTTON", "BTN Reset Counters pressed.");
        send_command("RSC01*");
    }
}

void create_main_screen(lv_obj_t *scr) {
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

    lv_obj_t *label_alarm = lv_label_create(alarm_box);
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
