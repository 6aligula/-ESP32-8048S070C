#include "translations.h"
#include <string.h>

// Tabla de traducciones
TranslationEntry translation_table[] = {
    {"LOGO", {"LOGO", "LOGO", "LOGO", "LOGO"}},
    {"MCV", {"MCV", "MCV", "MCV", "MCV"}},
    {"Inicio", {"Inicio", "Home", "Accueil", "Startseite"}},
    {"Ajustes", {"Ajustes", "Settings", "Paramètres", "Einstellungen"}},
    {"Atras", {"Atras", "Back", "Retour", "Zurück"}}
};

const int translation_table_size = sizeof(translation_table) / sizeof(TranslationEntry);

const char* get_translation(const char *key, Language lang) {
    for (int i = 0; i < translation_table_size; i++) {
        if (strcmp(translation_table[i].key, key) == 0) {
            return translation_table[i].translations[lang];
        }
    }
    return key; // Retorna la clave si no encuentra la traducción
}
