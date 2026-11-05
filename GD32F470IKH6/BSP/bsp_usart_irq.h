#ifndef __BSP_USART_IRQ_H
#define __BSP_USART_IRQ_H

#include "gd32f4xx.h"

#define BSP_USART_COM0        USART0
#define BSP_USART_IRQn        USART0_IRQn

void bsp_usart_irq_enable(void);
void bsp_usart_irq_disable_rx(void);
void bsp_usart_irq_disable_tx(void);

#endif
