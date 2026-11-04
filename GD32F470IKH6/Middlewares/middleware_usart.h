#ifndef __MIDDLEWARE_USART_H
#define __MIDDLEWARE_USART_H
#include "bsp_usart.h"
#include <stdio.h>

void middleware_usart_init(uint32_t baudrate);
void middleware_usart_send_string(char *str);

#endif
