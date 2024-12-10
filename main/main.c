#include "esp_check.h"

#include "bsp.h"

#include "esp_lvgl_port.h"

#include "driver/i2c_master.h"

#include "lv_examples.h"
#include "lv_demos.h"

#include "nav_panel.h"
#include "screens.h"
#include "settings_screen.h"
#include "driver/uart.h"
#include "uart_config.h"
#include "uart_utils.h"

#include "lvgl.h" // Asegúrate de incluir el encabezado de LVGL
LV_FONT_DECLARE(lv_font_montserrat_20); // Declarar la fuente habilitada


// codigo de navegación
lv_obj_t *main_screen;
lv_obj_t *settings_screen;

/* Callbacks para navegación */
void go_to_main_screen(void) {
    lv_scr_load(main_screen);
}

void go_to_settings_screen(void) {
    lv_scr_load(settings_screen);
}

void go_back(void) {
    lv_scr_load(main_screen); // Por simplicidad, siempre volvemos a la principal
}
//fin de la sección de código de navegación
// Tarea para manejar LVGL
void lvgl_task(void *pvParameter) {
    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10)); // Ajusta el retardo según sea necesario
    }
}

/* LCD settings */
#define APP_LCD_LVGL_FULL_REFRESH           (0)
#define APP_LCD_LVGL_DIRECT_MODE            (1)
#define APP_LCD_LVGL_AVOID_TEAR             (1)
#define APP_LCD_RGB_BOUNCE_BUFFER_MODE      (1)
#define APP_LCD_DRAW_BUFF_DOUBLE            (0)
#define APP_LCD_DRAW_BUFF_HEIGHT            (100)
#define APP_LCD_RGB_BUFFER_NUMS             (2)
#define APP_LCD_RGB_BOUNCE_BUFFER_HEIGHT    (10)

static esp_lcd_panel_handle_t lcd_panel = NULL;

static i2c_master_bus_handle_t my_bus = NULL;
static esp_lcd_panel_io_handle_t touch_io_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

static const char TAG[] = "rgb_panel_v2";

static esp_err_t app_lcd_init(esp_lcd_panel_handle_t *lp)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Initialize RGB panel");
    const esp_lcd_rgb_panel_config_t conf = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .timings = BSP_LCD_PANEL_TIMING(),
        .data_width = 16,
        .num_fbs = APP_LCD_RGB_BUFFER_NUMS,
#ifdef APP_LCD_RGB_BOUNCE_BUFFER_MODE
        .bounce_buffer_size_px = BSP_LCD_H_RES * APP_LCD_RGB_BOUNCE_BUFFER_HEIGHT,
#endif
        .hsync_gpio_num = BSP_LCD_GPIO_HSYNC,
        .vsync_gpio_num = BSP_LCD_GPIO_VSYNC,
        .de_gpio_num = BSP_LCD_GPIO_DE,
        .pclk_gpio_num = BSP_LCD_GPIO_PCLK,
        .disp_gpio_num = BSP_LCD_GPIO_DISP,
        .data_gpio_nums = BSP_LCD_GPIO_DATA(),
        .flags.fb_in_psram = 1,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_rgb_panel(&conf, lp),
                      err, TAG, "RGB init failed");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(*lp),
                      err, TAG, "LCD init failed");
    return ret;

err:
    if (*lp)
    {
        esp_lcd_panel_del(*lp);
    }
    return ret;
}

static esp_err_t app_touch_init(i2c_master_bus_handle_t *bus,
                                esp_lcd_panel_io_handle_t *tp_io,
                                esp_lcd_touch_handle_t *tp)
{
    if (!*bus)
    {
        ESP_LOGI(TAG, "creating i2c master bus");
        const i2c_master_bus_config_t i2c_conf = {
            .i2c_port = -1,
            .sda_io_num = BSP_TOUCH_GPIO_SDA,
            .scl_io_num = BSP_TOUCH_GPIO_SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = 1,
        };
        ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i2c_conf, bus),
                            TAG, "failed to create i2c master bus");
    }

    if (!*tp_io)
    {
        ESP_LOGI(TAG, "creating touch panel io");
        esp_lcd_panel_io_i2c_config_t tp_io_cfg =
            ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
        tp_io_cfg.scl_speed_hz = 400000;
        ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c_v2(*bus, &tp_io_cfg, tp_io),
                            TAG, "Failed to crate touch panel io");
    }

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,
        .y_max = BSP_LCD_V_RES,
        .rst_gpio_num = BSP_TOUCH_GPIO_RST,
        .int_gpio_num = BSP_TOUCH_GPIO_INT,
    };

    return esp_lcd_touch_new_i2c_gt911(*tp_io, &tp_cfg, tp);
}

static esp_err_t app_lvgl_init(esp_lcd_panel_handle_t lp, esp_lcd_touch_handle_t tp,
                               lv_display_t **lv_disp, lv_indev_t **lv_touch_indev)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,         /* LVGL task priority */
        .task_stack = 8192,         /* LVGL task stack size */
        .task_affinity = -1,        /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500,   /* Maximum sleep in LVGL task */
        .timer_period_ms = 5        /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    uint32_t buff_size = BSP_LCD_H_RES * APP_LCD_DRAW_BUFF_HEIGHT;
#if APP_LCD_LVGL_FULL_REFRESH || APP_LCD_LVGL_DIRECT_MODE
    buff_size = BSP_LCD_H_RES * BSP_LCD_V_RES;
#endif

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .panel_handle = lp,
        .buffer_size = buff_size,
        .double_buffer = APP_LCD_DRAW_BUFF_DOUBLE,
        .hres = BSP_LCD_H_RES,
        .vres = BSP_LCD_V_RES,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = false,
#if APP_LCD_LVGL_FULL_REFRESH
            .full_refresh = true,
#elif APP_LCD_LVGL_DIRECT_MODE
            .direct_mode = true,
#endif
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = false,
#endif
        }
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
#if APP_LCD_RGB_BOUNCE_BUFFER_MODE
            .bb_mode = true,
#else
            .bb_mode = false,
#endif
#if APP_LCD_LVGL_AVOID_TEAR
            .avoid_tearing = true,
#else
            .avoid_tearing = false,
#endif
        }
    };
    *lv_disp = lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = *lv_disp,
        .handle = tp,
    };
    *lv_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}

void app_main(void)
{
    // Configuración UART
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    // Configurar los parámetros para UART1
    uart_param_config(UART_PORT_NUM, &uart_config);

    // Asignar los pines GPIO17 y GPIO18 al UART1
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Instalar el controlador UART con un buffer de 1024 bytes
    uart_driver_install(UART_PORT_NUM, 1024, 0, 0, NULL, 0);
    
    // Inicializar UART Utils
    if (!uart_utils_init()) {
        ESP_LOGE("MAIN", "Failed to initialize UART utils");
        return;
    }

    // Inicializar LCD
    ESP_ERROR_CHECK(app_lcd_init(&lcd_panel));
    ESP_ERROR_CHECK(app_touch_init(&my_bus, &touch_io_handle, &touch_handle));
    ESP_ERROR_CHECK(app_lvgl_init(lcd_panel, touch_handle, &lvgl_disp, &lvgl_touch_indev));
    // Configuración de retroiluminación
    const gpio_config_t bk_light = {
        .pin_bit_mask = (1 << BSP_LCD_GPIO_BK_LIGHT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_light));
    gpio_set_level(BSP_LCD_GPIO_BK_LIGHT, BSP_LCD_BK_LIGHT_ON_LEVEL);

    // Inicializar pantallas
    main_screen = lv_obj_create(NULL);     // Crear objeto para la pantalla principal
    settings_screen = lv_obj_create(NULL); // Crear objeto para la pantalla de ajustes

    // Añadir contenido a las pantallas
    create_main_screen(main_screen);       // Inicializar contenido de la pantalla principal
    create_settings_screen(settings_screen); // Inicializar contenido de la pantalla de ajustes

    // Crear el panel de navegación en ambas pantallas
    lvgl_port_lock(0);
    create_nav_panel(main_screen, go_to_main_screen, go_to_settings_screen, go_back);
    create_nav_panel(settings_screen, go_to_main_screen, go_to_settings_screen, go_back);
    lvgl_port_unlock();

    // Mostrar la pantalla principal al inicio
    lv_scr_load(main_screen);

    // Crear tarea para manejar LVGL
    //xTaskCreate(lvgl_task, "lvgl_task", 4096, NULL, 5, NULL);

    // Crear tarea para recibir datos del UART
    xTaskCreate(uart_receive_task, "uart_receive_task", 4096, NULL, 10, NULL);
}
