#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

typedef enum {
    LANG_ES,
    LANG_EN,
    LANG_FR,
    LANG_DE,
    LANG_COUNT
} Language;

typedef struct {
    const char *key;
    const char *translations[LANG_COUNT];
} TranslationEntry;

extern TranslationEntry translation_table[];
extern const int translation_table_size;

const char* get_translation(const char *key, Language lang);

#endif // TRANSLATIONS_H
