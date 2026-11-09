#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "gd32f4xx.h"

#define COMn                             1U
#define EVAL_COM0                        USART0
#define EVAL_COM0_CLK                    RCU_USART0

#define EVAL_COM0_TX_PIN                 GPIO_PIN_9
#define EVAL_COM0_RX_PIN                 GPIO_PIN_10

#define EVAL_COM0_GPIO_PORT              GPIOA
#define EVAL_COM0_GPIO_CLK               RCU_GPIOA
#define EVAL_COM0_AF                     GPIO_AF_7

void bsp_usart_init(uint32_t com, uint32_t baudrate);

#endif
