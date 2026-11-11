#include "bsp_usart.h"

static const rcu_periph_enum USART_CLK[]      = { BSP_USART0_CLK };
static const rcu_periph_enum USART_GPIO_CLK[] = { BSP_USART0_GPIO_CLK };
static const uint32_t USART_GPIO_PORT[]       = { BSP_USART0_GPIO_PORT };
static const uint32_t USART_TX_PIN[]          = { BSP_USART0_TX_PIN };
static const uint32_t USART_RX_PIN[]          = { BSP_USART0_RX_PIN };
static const uint32_t USART_AF[]              = { BSP_USART0_AF };
static const uint32_t USART_PERIPH[]          = { BSP_USART0_PERIPH };
static const IRQn_Type USART_IRQn[]           = { BSP_USART0_IRQn };

#define USART_RX_BUF_SIZE 256
static uint8_t  rx_buffer[256];
static volatile uint16_t rx_len = 0;
static volatile uint8_t  rx_complete = 0;
static uint32_t g_printf_periph = USART0;

/*!
    \brief      初始化串口硬件参数
    \param[in]  usart_num: 串口编号
    \param[in]  usart_mode: 串口工作模式 (仅打印 或 中断收发)
    \param[in]  baudrate: 设定的波特率
    \param[out] 无
    \retval     无
*/
void bsp_usart_init(usart_typedef_enum usart_num, usartmode_typedef_enum usart_mode, uint32_t baudrate) {
    if (usart_num >= BSP_USART_COMn) return;
    uint32_t periph = USART_PERIPH[usart_num];

    rcu_periph_clock_enable(USART_GPIO_CLK[usart_num]);
    rcu_periph_clock_enable(USART_CLK[usart_num]);
    gpio_af_set(USART_GPIO_PORT[usart_num], USART_AF[usart_num], USART_TX_PIN[usart_num] | USART_RX_PIN[usart_num]);
    gpio_mode_set(USART_GPIO_PORT[usart_num], GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_TX_PIN[usart_num]);
    gpio_output_options_set(USART_GPIO_PORT[usart_num], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, USART_TX_PIN[usart_num]);
    gpio_mode_set(USART_GPIO_PORT[usart_num], GPIO_MODE_AF, GPIO_PUPD_PULLUP, USART_RX_PIN[usart_num]);

    usart_deinit(periph);
    usart_baudrate_set(periph, baudrate);
    usart_word_length_set(periph, USART_WL_8BIT);
    usart_stop_bit_set(periph, USART_STB_1BIT);
    usart_parity_config(periph, USART_PM_NONE);
    usart_receive_config(periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(periph, USART_TRANSMIT_ENABLE);

    if (usart_mode == BSP_USART_MODE_IRQ) {
        usart_interrupt_enable(periph, USART_INT_RBNE);
        nvic_irq_enable(USART_IRQn[usart_num], 2U, 0U);
    }
    usart_enable(periph);
    g_printf_periph = periph;
}

/*!
    \brief      在运行中途动态修改串口的波特率
    \param[in]  usart_num: 串口编号
    \param[in]  baudrate: 新的波特率
    \param[out] 无
    \retval     无
*/
void bsp_usart_set_baudrate(usart_typedef_enum usart_num, uint32_t baudrate) {
    if (usart_num >= BSP_USART_COMn) return;
    usart_disable(USART_PERIPH[usart_num]);
    usart_baudrate_set(USART_PERIPH[usart_num], baudrate);
    usart_enable(USART_PERIPH[usart_num]);
}

/*!
    \brief      串口0的中断服务函数 (自动覆盖启动文件里的空函数)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void USART0_IRQHandler(void) {
    if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET) {
        uint8_t data = (uint8_t)usart_data_receive(USART0);
        if(rx_len < USART_RX_BUF_SIZE - 1) rx_buffer[rx_len++] = data;
        rx_complete = 1;
    }
}

/*!
    \brief      获取串口中断接收到的数据
    \param[in]  usart_num: 串口编号
    \param[out] data: 存放读出数据的缓冲区
    \param[out] len: 实际读出的数据长度
    \retval     1表示有新数据，0表示暂无数据
*/
uint8_t bsp_usart_get_rx_data(usart_typedef_enum usart_num, uint8_t *data, uint16_t *len) {
    if (usart_num >= BSP_USART_COMn) return 0;
    if (rx_complete) {
        *len = rx_len;
        for (uint16_t i = 0; i < rx_len; i++) data[i] = rx_buffer[i];
        rx_len = 0; rx_complete = 0; return 1;
    }
    return 0;
}

/*!
    \brief      通过 DMA 方式发送一串数据
    \param[in]  usart_num: 串口编号
    \param[in]  data: 发送数据缓冲区首地址
    \param[in]  len: 发送数据长度(字节)
    \param[out] 无
    \retval     无
*/
void bsp_usart_send_dma(usart_typedef_enum usart_num, uint8_t *data, uint16_t len) {
    if (usart_num >= BSP_USART_COMn || len == 0) return;
    uint32_t periph = USART_PERIPH[usart_num];
    dma_single_data_parameter_struct dma_init;
    
    rcu_periph_clock_enable(RCU_DMA1);
    dma_deinit(DMA1, DMA_CH7);
    dma_single_data_para_struct_init(&dma_init);
    dma_init.direction = DMA_MEMORY_TO_PERIPH;
    dma_init.memory0_addr = (uint32_t)data;
    dma_init.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init.number = len;
    dma_init.periph_addr = (uint32_t)&USART_DATA(periph);
    dma_init.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(DMA1, DMA_CH7, &dma_init);
    dma_channel_subperipheral_select(DMA1, DMA_CH7, DMA_SUBPERI4);
    
    usart_dma_transmit_config(periph, USART_DENT_ENABLE);
    dma_channel_enable(DMA1, DMA_CH7);
    
    /* 1. 等待 DMA 搬运完成 */
    while(RESET == dma_flag_get(DMA1, DMA_CH7, DMA_INTF_FTFIF));
    dma_flag_clear(DMA1, DMA_CH7, DMA_INTF_FTFIF);
    
    /* 2. 【关键修复】等待 USART 将最后一个字节真正移位发送完成 */
    while(RESET == usart_flag_get(periph, USART_FLAG_TC));
}
/*!
    \brief      printf 底层重定向
    \param[in]  ch: 待发送字符
    \param[in]  f: 文件指针(无需关心)
    \param[out] 无
    \retval     发送的字符
*/
int fputc(int ch, FILE *f) {
    usart_data_transmit(g_printf_periph, (uint8_t)ch);
    while (RESET == usart_flag_get(g_printf_periph, USART_FLAG_TBE));
    return ch;
}