#include "bsp_usart_dma.h"

//void bsp_usart0_dma_init(void)
//{
//    dma_single_data_parameter_struct dma_init_struct;

//    /* 开启 DMA1 时钟 */
//    rcu_periph_clock_enable(RCU_DMA1);

//    // ========= TX DMA CH7：内存 → 外设 =========
//    dma_deinit(DMA1, BSP_USART0_TX_DMA_CH);
//    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
//    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
//    dma_init_struct.periph_addr = BSP_USART0_PERIPH_ADDR;
//    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
//    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
//    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
//    dma_init_struct.number = 0;
//    dma_init_struct.memory0_addr = 0;
//    dma_single_data_mode_init(DMA1, BSP_USART0_TX_DMA_CH, &dma_init_struct);
//    dma_channel_subperipheral_select(DMA1, BSP_USART0_TX_DMA_CH, DMA_SUBPERI4);
//    dma_circulation_disable(DMA1, BSP_USART0_TX_DMA_CH);

//    // ========= RX DMA CH2：外设 → 内存 =========
//    dma_deinit(DMA1, BSP_USART0_RX_DMA_CH);
//    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
//    dma_single_data_mode_init(DMA1, BSP_USART0_RX_DMA_CH, &dma_init_struct);
//    dma_channel_subperipheral_select(DMA1, BSP_USART0_RX_DMA_CH, DMA_SUBPERI4);
//    dma_circulation_disable(DMA1, BSP_USART0_RX_DMA_CH);

//    /* 开启串口 DMA 收发使能 */
//    usart_dma_transmit_config(USART0, USART_DENT_ENABLE);
//    usart_dma_receive_config(USART0, USART_DENT_ENABLE);
//}

//void bsp_usart0_dma_tx_start(uint8_t *p_buf, uint16_t len)
//{
//    dma_memory_address_config(DMA1, BSP_USART0_TX_DMA_CH, DMA_MEMORY_0, (uint32_t)p_buf);
//    dma_transfer_number_config(DMA1, BSP_USART0_TX_DMA_CH, len);
//    dma_channel_enable(DMA1, BSP_USART0_TX_DMA_CH);
//}

//void bsp_usart0_dma_rx_start(uint8_t *p_buf, uint16_t len)
//{
//    dma_memory_address_config(DMA1, BSP_USART0_RX_DMA_CH, DMA_MEMORY_0, (uint32_t)p_buf);
//    dma_transfer_number_config(DMA1, BSP_USART0_RX_DMA_CH, len);

//    dma_channel_enable(DMA1, BSP_USART0_RX_DMA_CH);
//}

//FlagStatus bsp_usart0_dma_tx_is_complete(void)
//{
//    return dma_flag_get(DMA1, BSP_USART0_TX_DMA_CH, DMA_INTF_FTFIF);
//}

//FlagStatus bsp_usart0_dma_rx_is_complete(void)
//{
//    return dma_flag_get(DMA1, BSP_USART0_RX_DMA_CH, DMA_INTF_FTFIF);
//}

//void bsp_usart0_dma_flag_clear(void)
//{
//    dma_flag_clear(DMA1, BSP_USART0_TX_DMA_CH, DMA_INTF_FTFIF);
//    dma_flag_clear(DMA1, BSP_USART0_RX_DMA_CH, DMA_INTF_FTFIF);
//}
