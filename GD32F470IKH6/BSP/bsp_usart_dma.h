#ifndef __BSP_USART_DMA_H
#define __BSP_USART_DMA_H

#include "gd32f4xx.h"

// 串口0 + DMA 资源定义
#define BSP_USART0_TX_DMA_CH    DMA_CH7
#define BSP_USART0_RX_DMA_CH    DMA_CH2
#define BSP_USART0_PERIPH_ADDR  ((uint32_t)0x40011004U)

void bsp_usart0_dma_init(void);
void bsp_usart0_dma_tx_start(uint8_t *p_buf, uint16_t len);
void bsp_usart0_dma_rx_start(uint8_t *p_buf, uint16_t len);
FlagStatus bsp_usart0_dma_tx_is_complete(void);
FlagStatus bsp_usart0_dma_rx_is_complete(void);
void bsp_usart0_dma_flag_clear(void);

#endif
