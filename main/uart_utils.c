// uart_utils.c
#include "uart_utils.h"
#include "esp_log.h"
#include "driver/uart.h"
#include <string.h>
#include "uart_config.h"
#include "freertos/semphr.h"

#define MAX_UART_HANDLERS 10

typedef void (*uart_data_handler_t)(const char *);

static uart_data_handler_t data_handlers[MAX_UART_HANDLERS] = {0};

// Mutex para proteger el acceso a la lista de handlers
static SemaphoreHandle_t handlers_mutex = NULL;

// Función pública para inicializar los handlers
bool uart_utils_init(void) {
    if (handlers_mutex == NULL) {
        handlers_mutex = xSemaphoreCreateMutex();
        if (handlers_mutex == NULL) {
            ESP_LOGE("UART_UTILS", "Failed to create handlers mutex");
            return false;
        }
        ESP_LOGI("UART_UTILS", "Handlers mutex initialized");
    }

    // Desactivar el modo eco
    const char *disable_echo_cmd = "ATE0\r\n";
    if (uart_write_bytes(UART_PORT_NUM, disable_echo_cmd, strlen(disable_echo_cmd)) == strlen(disable_echo_cmd)) {
        ESP_LOGI("UART_UTILS", "Modo eco desactivado: %s", disable_echo_cmd);
    } else {
        ESP_LOGE("UART_UTILS", "No se pudo enviar el comando para desactivar el modo eco");
        return false;
    }

    return true;
}


bool uart_register_handler(uart_data_handler_t handler) {
    if (handlers_mutex == NULL) {
        ESP_LOGE("UART_UTILS", "handlers_mutex not initialized");
        return false;
    }

    bool registered = false;
    if (xSemaphoreTake(handlers_mutex, portMAX_DELAY) == pdTRUE) {
        for (int i = 0; i < MAX_UART_HANDLERS; i++) {
            if (data_handlers[i] == NULL) {
                data_handlers[i] = handler;
                ESP_LOGI("UART_UTILS", "Handler registrado en slot %d: %p", i, (void *)handler);
                registered = true;
                break;
            }
        }
        xSemaphoreGive(handlers_mutex);
    }

    if (!registered) {
        ESP_LOGW("UART_UTILS", "No se pudo registrar el handler, máximo alcanzado");
    }
    return registered;
}

bool uart_unregister_handler(uart_data_handler_t handler) {
    if (handlers_mutex == NULL) {
        ESP_LOGE("UART_UTILS", "handlers_mutex not initialized");
        return false;
    }

    bool unregistered = false;
    if (xSemaphoreTake(handlers_mutex, portMAX_DELAY) == pdTRUE) {
        for (int i = 0; i < MAX_UART_HANDLERS; i++) {
            if (data_handlers[i] == handler) {
                data_handlers[i] = NULL;
                ESP_LOGI("UART_UTILS", "Handler desregistrado del slot %d: %p", i, (void *)handler);
                unregistered = true;
                break;
            }
        }
        xSemaphoreGive(handlers_mutex);
    }

    if (!unregistered) {
        ESP_LOGW("UART_UTILS", "Handler no encontrado para desregistrar: %p", (void *)handler);
    }
    return unregistered;
}

void send_command(const char *command) {
    uart_write_bytes(UART_PORT_NUM, command, strlen(command));
    ESP_LOGI("UART", "Enviado: %s", command);
}

void uart_receive_task(void *arg) {
    char rx_buffer[128];
    const TickType_t delay_ticks = pdMS_TO_TICKS(100); // 100 ms en ticks

    while (true) {
        int length = uart_read_bytes(UART_PORT_NUM, (uint8_t *)rx_buffer, sizeof(rx_buffer) - 1, pdMS_TO_TICKS(1000));
        if (length > 0) {
            rx_buffer[length] = '\0'; // Asegurar terminación de cadena
            ESP_LOGI("UART", "Recibido: %s", rx_buffer);

            // Llama a todos los handlers registrados
            if (handlers_mutex != NULL) {
                if (xSemaphoreTake(handlers_mutex, portMAX_DELAY) == pdTRUE) {
                    for (int i = 0; i < MAX_UART_HANDLERS; i++) {
                        if (data_handlers[i] != NULL) {
                            ESP_LOGI("UART_UTILS", "Llamando al handler en slot %d: %p", i, (void *)data_handlers[i]);
                            data_handlers[i](rx_buffer);
                        }
                    }
                    xSemaphoreGive(handlers_mutex);
                }
            } else {
                ESP_LOGW("UART_UTILS", "handlers_mutex is NULL, no handlers to call");
            }
        }
        vTaskDelay(delay_ticks);
    }
}
