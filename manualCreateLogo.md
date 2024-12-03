## Manual: Proceso para Colocar un Logo en un Panel usando LVGL

Este manual detalla los pasos necesarios para colocar un logo en un panel de navegación creado con LVGL, incluyendo el cálculo de medidas, generación del archivo `.c` y configuración final.

---

### **1. Generar el archivo de imagen en formato `.c`

#### Pasos para usar la herramienta de conversión:
1. Accede al **Image Converter de LVGL**:  
   URL: [https://lvgl.io/tools/imageconverter](https://lvgl.io/tools/imageconverter)

2. **Sube tu imagen**:
   - Haz clic en **"Select image file(s)"** y selecciona el archivo de tu logo (formato `.jpg`, `.png`, `.bmp`, etc.).
   - Asegúrate de que el logo tenga un tamaño razonable (por ejemplo, entre 100x100 px a 300x300 px).

3. **Configura las opciones de conversión**:
   - **Color format:** Selecciona `True color with alpha` (para mantener la transparencia si es necesaria).
   - **Output format:** Elige `C array`.

4. **Genera el archivo**:
   - Descarga el archivo `.c` generado.

5. **Incluye el archivo en tu proyecto**:
   - Copia el archivo `.c` a la carpeta de tu proyecto y añádelo al sistema de compilación (por ejemplo, `CMakeLists.txt` si usas CMake o `idf_component_register` si usas ESP-IDF).

---

### **2. Cálculo de medidas para la imagen**

#### Parámetros relevantes:
- **Tamaño del panel (`nav_panel`)**:
  - En este ejemplo, el panel tiene un alto de `70 px` y un ancho del `100%` de la pantalla.
- **Tamaño sugerido del logo**:
  - **Altura:** Entre el 60-70% del alto del panel (aproximadamente `50 px` en este caso).
  - **Ancho:** Proporcional al diseño del logo, pero no debe exceder el 20-30% del ancho del panel.

#### Fórmula para escalar el logo:
El tamaño de la imagen en LVGL se ajusta mediante `lv_img_set_zoom()`. Este método escala el logo según un valor relativo, donde:
- **256 = 100%** del tamaño original.

Fórmula para calcular el zoom:
```text
zoom_factor = (tamaño_deseado / tamaño_original) * 256
```

Ejemplo:
- **Tamaño original de la imagen:** 100x100 px (subido y convertido).
- **Altura deseada:** 50 px.

Cálculo:
```text
zoom_factor = (50 / 100) * 256 = 128
```

---

### **3. Configuración en el código**

#### Archivo de encabezado `logo.h`:
Asegúrate de que tu archivo `.c` generado esté acompañado de un archivo de encabezado (`logo.h`) con la declaración del recurso:
```c
#ifndef LOGO_H
#define LOGO_H

#include "lvgl.h"

extern const lv_img_dsc_t logo;

#endif // LOGO_H
```

#### Código en el archivo fuente:
1. **Crear el panel** y agregar el logo:
   ```c
   #include "nav_panel.h"
   #include "logo.h" // Archivo generado con la herramienta

   lv_obj_t *create_nav_panel(lv_obj_t *parent, nav_callback_t home_cb, nav_callback_t settings_cb, nav_callback_t back_cb) {
       // Crear el panel de navegación
       lv_obj_t *nav_panel = lv_obj_create(parent);
       lv_obj_set_size(nav_panel, lv_pct(100), 70);  // Ancho 100%, alto 70px
       lv_obj_align(nav_panel, LV_ALIGN_TOP_MID, 0, 0);

       // Agregar el logo como imagen al panel
       lv_obj_t *logo_img = lv_img_create(nav_panel);
       lv_img_set_src(logo_img, &logo); // Descriptor `logo` del archivo generado
       lv_obj_align(logo_img, LV_ALIGN_LEFT_MID, 10, 0); // Margen de 10px desde la izquierda
       lv_img_set_zoom(logo_img, 128); // Escala al 50% del tamaño original

       return nav_panel;
   }
   ```

2. **Notas importantes**:
   - El margen izquierdo (`10 px`) puede ajustarse en el tercer parámetro de `lv_obj_align`.
   - Usa `lv_img_set_zoom()` para escalar la imagen si es más grande o más pequeña de lo necesario.

---

### **3. Configuración del sistema de compilación**

#### Para ESP-IDF:
En el archivo `CMakeLists.txt`, agrega el archivo `.c` generado:
```cmake
idf_component_register(SRCS "main.c" "nav_panel.c" "logo.c"
                       INCLUDE_DIRS .
                       REQUIRES lvgl)
```

### **4. Verificar y ajustar**

1. **Depuración visual**:
   - Usa colores de fondo en el panel y en el logo para verificar el alineamiento:
     ```c
     lv_obj_set_style_bg_color(nav_panel, lv_palette_main(LV_PALETTE_BLUE), 0);
     lv_obj_set_style_bg_opa(nav_panel, LV_OPA_50, 0);

     lv_obj_set_style_bg_color(logo_img, lv_palette_main(LV_PALETTE_RED), 0);
     lv_obj_set_style_bg_opa(logo_img, LV_OPA_50, 0);
     ```

2. **Ajustar escalado**:
   Si el logo sigue siendo demasiado grande o pequeño, ajusta `lv_img_set_zoom()` con un nuevo `zoom_factor`.

---