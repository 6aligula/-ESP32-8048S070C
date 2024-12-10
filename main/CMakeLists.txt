idf_component_register(SRCS "uart_utils.c" "main.c" "nav_panel.c" "screens.c" "logo.c" "settings_screen.c" "uart_utils.c"
                    INCLUDE_DIRS .
                    REQUIRES esp_lcd driver)
