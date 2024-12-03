#include "settings_screen.h"
#include "lvgl.h"

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
