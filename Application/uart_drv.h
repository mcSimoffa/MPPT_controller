#ifndef __UART_DRV_H
#define __UART_DRV_H

#include <string.h>

void uart_drv_Init(void);
void uart_drv_send(uint8_t *data, size_t size);
void uart_it_handler(void);

void log_print(const uint8_t *data);

#endif // __UART_DRV_H
