#include "nav_panel.h"
#include "translations.h"  // Incluir soporte de traducciones
#include <string.h>        // Para strcmp
#include "esp_log.h"       // Para ESP_LOGI
#include <inttypes.h>

// Definiciones de variables globales como static
static lv_obj_t *logo;
static lv_obj_t *title;
static lv_obj_t *btn_home;
static lv_obj_t *btn_settings;
static lv_obj_t *btn_back;
static lv_obj_t *label_home;
static lv_obj_t *label_settings;
static lv_obj_t *label_back;
static lv_obj_t *nav_panel = NULL;

// Variable global para el idioma actual
static Language current_language = LANG_ES; // Idioma por defecto

// Callback para manejar el cambio de idioma desde el dropdown
static void language_dropdown_event_cb(lv_event_t *e) {
    // Obtener el objeto dropdown
    lv_obj_t *dropdown = lv_event_get_target(e);
    // Obtener el panel de navegación desde el user_data
    NavPanel *panel = (NavPanel*)lv_event_get_user_data(e);

    if (!panel) {
        ESP_LOGE(TAG, "Panel de navegación no válido");
        return;
    }

    // Obtener el texto seleccionado
    char selected_text[10];
    lv_dropdown_get_selected_str(dropdown, selected_text, sizeof(selected_text));

    // Mapear texto seleccionado al enum Language
    if (strcmp(selected_text, "Esp") == 0) panel->current_language = LANG_ES;
    else if (strcmp(selected_text, "Eng") == 0) panel->current_language = LANG_EN;
    else if (strcmp(selected_text, "Fran") == 0) panel->current_language = LANG_FR;
    else if (strcmp(selected_text, "Ger") == 0) panel->current_language = LANG_DE;
    else {
        ESP_LOGW(TAG, "Idioma seleccionado desconocido: %s", selected_text);
        return;
    }

    ESP_LOGI(TAG, "Idioma cambiado a: %d", panel->current_language);

    // Actualizar textos
    const char* new_logo = get_translation("LOGO", panel->current_language);
    ESP_LOGI(TAG, "Nuevo LOGO: %s", new_logo);
    lv_label_set_text(panel->logo, new_logo);

    const char* new_title = get_translation("MCV", panel->current_language);
    ESP_LOGI(TAG, "Nuevo MCV: %s", new_title);
    lv_label_set_text(panel->title, new_title);

    const char* new_home = get_translation("Inicio", panel->current_language);
    ESP_LOGI(TAG, "Nuevo Inicio: %s", new_home);
    lv_label_set_text(panel->label_home, new_home);

    const char* new_settings = get_translation("Ajustes", panel->current_language);
    ESP_LOGI(TAG, "Nuevo Ajustes: %s", new_settings);
    lv_label_set_text(panel->label_settings, new_settings);

    const char* new_back = get_translation("Atras", panel->current_language);
    ESP_LOGI(TAG, "Nuevo Atras: %s", new_back);
    lv_label_set_text(panel->label_back, new_back);

    // Actualizar el diseño si es necesario
    lv_obj_update_layout(panel->nav_panel);
    lv_refr_now(NULL);
}


// Función para crear el panel de navegación
lv_obj_t *create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb) {
    // Asignar memoria para NavPanel
    NavPanel *panel = malloc(sizeof(NavPanel));
    if (!panel) {
        ESP_LOGE(TAG, "No se pudo asignar memoria para NavPanel");
        return NULL;
    }

    panel->current_language = LANG_ES; // Idioma por defecto
    // Crear el panel de navegación
    nav_panel = lv_obj_create(parent);
    lv_obj_set_size(nav_panel, lv_pct(100), 70);  // Ancho 100%, alto 70px
    lv_obj_align(nav_panel, LV_ALIGN_TOP_MID, 0, 0);

    // Estilo del panel
    lv_obj_set_style_bg_color(nav_panel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(nav_panel, LV_OPA_COVER, LV_PART_MAIN);

    // Agregar el logo al panel
    logo = lv_label_create(nav_panel);
    lv_label_set_text(logo, get_translation("LOGO", current_language));
    lv_obj_set_style_text_color(logo, lv_color_hex(0xFFFFFF), 0); // Blanco
    lv_obj_set_style_text_font(logo, &lv_font_montserrat_14, 0);
    lv_obj_align(logo, LV_ALIGN_LEFT_MID, 10, 0);

    // Dropdown para seleccionar idioma
    lv_obj_t *dropdown_language = lv_dropdown_create(nav_panel);
    lv_dropdown_set_options_static(dropdown_language, "Esp\nEng\nFran\nGer");
    lv_obj_set_size(dropdown_language, 100, 30);
    lv_obj_align(dropdown_language, LV_ALIGN_LEFT_MID, 100, 0);
    lv_obj_add_event_cb(dropdown_language, language_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Agregar el título al panel
    title = lv_label_create(nav_panel);
    lv_label_set_text(title, get_translation("MCV", current_language));
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 160, 0);

    // Botón de inicio
    btn_home = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_home, 80, 30);
    lv_obj_align(btn_home, LV_ALIGN_CENTER, -120, 0);
    label_home = lv_label_create(btn_home);
    lv_label_set_text(label_home, get_translation("Inicio", current_language));
    lv_obj_add_event_cb(btn_home, (lv_event_cb_t)home_cb, LV_EVENT_CLICKED, NULL);

    // Botón de ajustes
    btn_settings = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_settings, 80, 30);
    lv_obj_align(btn_settings, LV_ALIGN_CENTER, 20, 0);
    label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, get_translation("Ajustes", current_language));
    lv_obj_align(label_settings, LV_ALIGN_CENTER, -3, 0);
    lv_obj_add_event_cb(btn_settings, (lv_event_cb_t)settings_cb, LV_EVENT_CLICKED, NULL);

    // Botón de atrás
    btn_back = lv_btn_create(nav_panel);
    lv_obj_set_size(btn_back, 80, 30);
    lv_obj_align(btn_back, LV_ALIGN_RIGHT_MID, -10, 0);
    label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, get_translation("Atras", current_language));
    lv_obj_add_event_cb(btn_back, (lv_event_cb_t)back_cb, LV_EVENT_CLICKED, NULL);

    return nav_panel;
}
