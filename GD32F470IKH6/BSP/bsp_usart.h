#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "gd32f4xx.h"

/* 当前硬件实际接了几路串口（决定查表数组长度） */
#define BSP_USART_COM_NUM       1U

/* DMA 接收缓冲区大小 */
#define DMA_RX_BUF_SIZE         256U

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

/* ---------------- DMA 通道映射（依据 GD32F4xx 官方 demo V3.3.3） ----------------
 * USART0_RX -> DMA1_CH2 , DMA_SUBPERI4
 * USART0_TX -> DMA1_CH7 , DMA_SUBPERI4
 * 若你的具体型号手册不同，只改这几行即可。
 */
#define BSP_USART0_DMA          DMA1
#define BSP_USART0_RX_DMA_CH    DMA_CH2
#define BSP_USART0_TX_DMA_CH    DMA_CH7
#define BSP_USART0_DMA_SUBPERI  DMA_SUBPERI4
#define BSP_USART0_TX_DMA_IRQn  DMA1_Channel7_IRQn

/* ---------------- DMA 共享变量（.c 定义，这里 extern） ---------------- */
extern uint8_t           dma_rx_buffer[DMA_RX_BUF_SIZE];
extern volatile uint16_t dma_rx_len;
extern volatile uint8_t  dma_rx_complete;
extern volatile uint8_t  dma_tx_complete;

/* ---------------- 函数原型 ---------------- */
void bsp_usart_init(usart_typedef_enum usart_num,
                    usartmode_typedef_enum usart_mode,
                    uint32_t baudrate);

void bsp_usart_dma_rx_init(void);
void bsp_usart_dma_tx_start(uint8_t *data, uint16_t len);

#endif /* __BSP_USART_H */