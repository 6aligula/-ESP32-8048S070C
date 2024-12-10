// uart_utils.h
#ifndef UART_UTILS_H
#define UART_UTILS_H

#include <stdint.h>
#include <stdbool.h>

// Definición del tipo de handler
typedef void (*uart_data_handler_t)(const char *);

// Función para inicializar UART Utils
bool uart_utils_init(void);

// Función para registrar un handler
bool uart_register_handler(uart_data_handler_t handler);

// Función para desregistrar un handler
bool uart_unregister_handler(uart_data_handler_t handler);

// Función para enviar comandos
void send_command(const char *command);

// Declaración de la tarea de recepción UART
void uart_receive_task(void *arg);

#endif // UART_UTILS_H
