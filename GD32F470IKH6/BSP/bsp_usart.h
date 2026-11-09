#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "gd32f4xx.h"

/* 当前硬件实际接了几路串口（决定查表数组长度） */
#define BSP_USART_COM_NUM       1U

typedef enum
{
    BSP_USART_COM0 = 0,
    BSP_USART_COM1 = 1,
    BSP_USART_COM2 = 2
} usart_typedef_enum;

typedef enum
{
    BSP_USART_MODE_PRINTF = 0,   /* 轮询 + printf 重定向 */
    BSP_USART_MODE_IRQ    = 1,   /* 中断收发 */
    BSP_USART_MODE_DMA    = 2    /* DMA 收发 */
} usartmode_typedef_enum;

/* ---------------- USART0 : PA9(TX) / PA10(RX), AF7 ---------------- */
#define BSP_USART0_PERIPH       USART0
#define BSP_USART0_CLK          RCU_USART0
#define BSP_USART0_GPIO_PORT    GPIOA
#define BSP_USART0_GPIO_CLK     RCU_GPIOA
#define BSP_USART0_TX_PIN       GPIO_PIN_9
#define BSP_USART0_RX_PIN       GPIO_PIN_10
#define BSP_USART0_AF           GPIO_AF_7
#define BSP_USART0_IRQn         USART0_IRQn

void    bsp_usart_init(usart_typedef_enum usart_num,
                       usartmode_typedef_enum usart_mode,
                       uint32_t baudrate);

//void    bsp_usart_send_byte(usart_typedef_enum usart_num, uint8_t byte);
//uint8_t bsp_usart_recv_byte(usart_typedef_enum usart_num);

#endif