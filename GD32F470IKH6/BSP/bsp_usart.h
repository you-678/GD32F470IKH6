#ifndef __BSP_USART_H
#define __BSP_USART_H
#include "gd32f4xx.h"

#define BSP_USART_COM0        USART0
#define BSP_USART_GPIO_PORT   GPIOA
#define BSP_USART_TX_PIN      GPIO_PIN_9
#define BSP_USART_RX_PIN      GPIO_PIN_10
#define BSP_USART_AF          GPIO_AF_7
#define BSP_USART_IRQn        USART0_IRQn

void bsp_usart_init(uint32_t baudrate);

void bsp_usart_send_byte(uint8_t dat);
uint8_t bsp_usart_recv_byte(void);

#endif
