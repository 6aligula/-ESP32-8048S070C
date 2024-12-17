#include "screens.h"
#include <string.h> // Para strlen
#include "driver/uart.h"
#include "uart_config.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "uart_utils.h"

#define NUM_ERROR_BITS 32

static lv_obj_t *label_temp1;
static lv_obj_t *label_temp2;
static lv_obj_t *label_volume;
static lv_obj_t *label_alarm;
static lv_obj_t *label_tot;
static lv_obj_t *label_lot;
static lv_obj_t *label_status;


// Definir la estructura para los datos UART
typedef struct {
    float t1;
    float t2;
    int vol;
	int errCode;           
	int statusCode;
	int cntLot;
	int cntTot;
    const char *status;  // Puntero a cadena
} uart_data_t;

static uart_data_t latest_data;

char *error_messages[] = {"Error en Home jeringa", "Error carga jeringa", "Error dosis jeringa", "Error paso transporte", 
						 "Error precarga transporte","Error home transporte", "Error Tapa ON", "Error Tapa OFF",
								"Error home tapa", "Error lectura Calor", "Error lectura frio", "Err 12", "Err 13", "Err 14", "Err 15", "Err 16",
								"Falta presion aire", "Paro de Emergencia", "Puerta frontal abierta", "Seguridades NOK", "Puerta lateral abierta"};


bool decode_bit(int byte, int bit_position)
{ 
	return (byte & (1 << bit_position)) != 0; 
}


void display_errors(int error_byte, int status_byte)
{ 
	char error_text[256] = ""; 
	
	for (int i = 0; i < NUM_ERROR_BITS; i++)
	{
		if (decode_bit(error_byte, i))
		{
			strcat(error_text, error_messages[i]); 
			strcat(error_text, "\n");
		}
	} 
	
	if (strlen(error_text) == 0)
	{
		strcpy(error_text, "No se detectaron errores.");
	} 
	
	lv_label_set_text(label_alarm, error_text);
	
	// STATUS
	
	if (decode_bit(status_byte, 0))
	{
	  lv_label_set_text(label_status, "MARCHA");
	  lv_obj_set_style_text_color(label_status, lv_color_hex(0x000000), 0);
	}
	else
	{
	  lv_label_set_text(label_status, "PARO");
	  lv_obj_set_style_text_color(label_status, lv_color_hex(0xff0000), 0);
	}
		
}


// Función de callback para actualizar las etiquetas
static void update_labels_callback(void *param) {
    uart_data_t *data = (uart_data_t *)param;
    lv_label_set_text_fmt(label_temp1, "T1: %.2f °C", data->t1);
    lv_label_set_text_fmt(label_temp2, "T2: %.2f °C", data->t2);
    lv_label_set_text_fmt(label_volume, "Volumen: %d ml", data->vol);
    lv_label_set_text_fmt(label_tot, "Tot: %u", data->cntTot);
	lv_label_set_text_fmt(label_lot, "Lot: %u", data->cntLot);
	
	display_errors(data->errCode, data->statusCode);
	
}



static void screen_data_handler(const char *data) {
    ESP_LOGI("SCREEN", "Handler invocado con: %s", data);
    if (strncmp(data, "DATA", 4) == 0) {
        float t1, t2;
        //int vol;
		float vol;
		unsigned int cntTot, cntLot, errCode, statusCode;
		//unsigned int errores = 0;
        if (sscanf(data, "DATA:TH=%f;TC=%f;VOL=%f;TOT=%u;LOT=%u;ERR=%u;STA=%u;", &t1, &t2, &vol, &cntTot, &cntLot, &errCode, &statusCode) == 7) {
            ESP_LOGI("SCREEN", "-->DECODED: - T1: %.1f, T2: %.1f, Vol: %.1f, Tot: %u, Lot: %u, ErrCode: %u, StatusCode: %u", t1, t2, vol, cntTot, cntLot, errCode, statusCode);

            // Almacenar los datos más recientes
            latest_data.t1 = t1;
            latest_data.t2 = t2;
            latest_data.vol = vol;
			latest_data.cntLot = cntLot;
            latest_data.cntTot = cntTot;			
			//latest_data.errCode = errCode;
			latest_data.errCode = errCode;
			latest_data.statusCode = statusCode;
			
			latest_data.status = "RUN!!!";

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
        send_command("STA*");
    } else if (strcmp(btn_label, "STOP") == 0) {
        ESP_LOGI("BUTTON", "Botón Stop presionado.");
        send_command("STO*");
    } else if (strcmp(btn_label, "RESET") == 0) {
        ESP_LOGI("BUTTON", "Botón Reset presionado.");
        send_command("RES*");
    } else if (strcmp(btn_label, "RST CNT") == 0) {
        ESP_LOGI("BUTTON", "BTN Reset Counters pressed.");
        send_command("RSC*");
	} else if (strcmp(btn_label, "PRELOAD") == 0) {
        ESP_LOGI("BUTTON", "BTN Preload.");
        send_command("PRE*");
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
    lv_label_set_text(label_temp1, "T1: - °C");
    lv_obj_set_style_text_color(label_temp1, lv_color_hex(0x333333), 0); // Naranja
    lv_obj_set_style_text_font(label_temp1, &lv_font_montserrat_20, 0);
    lv_obj_align(label_temp1, LV_ALIGN_TOP_LEFT, 50, 130); // Alineado a la izquierda

    label_temp2 = lv_label_create(scr);
    lv_label_set_text(label_temp2, "T2: - °C");
    lv_obj_set_style_text_color(label_temp2, lv_color_hex(0x333333), 0); // Naranja
    lv_obj_set_style_text_font(label_temp2, &lv_font_montserrat_20, 0);
    lv_obj_align(label_temp2, LV_ALIGN_TOP_LEFT, 50, 160); // Alineado debajo del primero

    // Volumen
    label_volume = lv_label_create(scr);
    lv_label_set_text(label_volume, "Volumen: - ml");
    lv_obj_set_style_text_color(label_volume, lv_color_hex(0x333333), 0); // Naranja
    lv_obj_set_style_text_font(label_volume, &lv_font_montserrat_20, 0);
    lv_obj_align(label_volume, LV_ALIGN_TOP_LEFT, 50, 190); // Alineado debajo del segundo

	// Status
    label_status = lv_label_create(scr);
    lv_label_set_text(label_status, "- - - -");
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x000000), 0); // negro
    lv_obj_set_style_text_font(label_status, &lv_font_montserrat_20, 0);
    lv_obj_align(label_status, LV_ALIGN_TOP_RIGHT, -290, 175);

    // Contadores
    lv_obj_t *counters_box = lv_obj_create(scr);
    lv_obj_set_size(counters_box, 400, 75);
    lv_obj_set_style_bg_color(counters_box, lv_color_hex(0xDDA0DD), 0); // Color púrpura claro
    lv_obj_align(counters_box, LV_ALIGN_TOP_RIGHT, -10, 90); // Alineado a la derecha

    // TOT
	label_tot = lv_label_create(counters_box);
    lv_label_set_text(label_tot, "TOT: -");
    lv_obj_set_style_text_font(label_tot, &lv_font_montserrat_20, 0);
    lv_obj_align(label_tot, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado dentro del contenedor
	
	// LOT
	label_lot = lv_label_create(counters_box);
    lv_label_set_text(label_lot, "LOT: -");
    lv_obj_set_style_text_font(label_lot, &lv_font_montserrat_20, 0);
    lv_obj_align(label_lot, LV_ALIGN_TOP_LEFT, 200, 10); // Alineado debajo del primer contador

    // Alarmas y errores
    lv_obj_t *alarm_box = lv_obj_create(scr);
	// alarm_box = lv_obj_create(scr);
    lv_obj_set_size(alarm_box, 400, 200);
    lv_obj_set_style_bg_color(alarm_box, lv_color_hex(0xADD8E6), 0); // Azul claro
    lv_obj_align(alarm_box, LV_ALIGN_BOTTOM_RIGHT, -10, -75); // Posicionado en la parte inferior

	label_alarm = lv_label_create(alarm_box);
    lv_label_set_text(label_alarm, "Alarmas / Errores:\n- Ninguna");
    lv_obj_set_style_text_font(label_alarm, &lv_font_montserrat_20, 0);
    lv_obj_align(label_alarm, LV_ALIGN_TOP_LEFT, 10, 10); // Alineado dentro del cuadro de alarmas

    // Crear botones y asignar el callback genérico
	
	#define NumButtons 5
	
    struct {
        const char *label;
        lv_color_t color;
        lv_align_t align;
        int offset_x;
    } buttons[] = {
        {"START", lv_color_hex(0x32CD32), LV_ALIGN_BOTTOM_LEFT, +10},
        {"STOP", lv_color_hex(0xFF4500), LV_ALIGN_BOTTOM_LEFT, +150},
        {"RESET", lv_color_hex(0xFFA500), LV_ALIGN_BOTTOM_RIGHT, -150},
		{"RST CNT", lv_color_hex(0x333333), LV_ALIGN_BOTTOM_RIGHT, -10},
		{"PRELOAD", lv_color_hex(0x333333), LV_ALIGN_BOTTOM_LEFT, +300},
    };

    for (int i = 0; i < NumButtons; i++) {
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


}


