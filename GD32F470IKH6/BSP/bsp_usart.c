#include "bsp_usart.h"
#include "gd32f4xx_dma.h"      /* dma_single_data_parameter_struct 等类型在这里 */

/* 兼容不同版本库的 USART DMA 使能宏名（V3.x 用 TRANSMIT/RECEIVE_DMA_xxx，旧版用 DENT/DENR） */
#ifndef USART_TRANSMIT_DMA_ENABLE
#define USART_TRANSMIT_DMA_ENABLE   USART_DENT_ENABLE
#endif
#ifndef USART_TRANSMIT_DMA_DISABLE
#define USART_TRANSMIT_DMA_DISABLE  USART_DENT_DISABLE
#endif
#ifndef USART_RECEIVE_DMA_ENABLE
#define USART_RECEIVE_DMA_ENABLE    USART_DENR_ENABLE
#endif
#ifndef USART_RECEIVE_DMA_DISABLE
#define USART_RECEIVE_DMA_DISABLE   USART_DENR_DISABLE
#endif

#define COM_COUNT   (sizeof(USART_CLK) / sizeof(USART_CLK[0]))

static const rcu_periph_enum USART_CLK[]      = { BSP_USART0_CLK };
static const rcu_periph_enum USART_GPIO_CLK[] = { BSP_USART0_GPIO_CLK };
static const uint32_t       USART_GPIO_PORT[] = { BSP_USART0_GPIO_PORT };
static const uint32_t       USART_TX_PIN[]    = { BSP_USART0_TX_PIN };
static const uint32_t       USART_RX_PIN[]    = { BSP_USART0_RX_PIN };
static const uint32_t       USART_AF[]        = { BSP_USART0_AF };
static const uint32_t       USART_PERIPH[]    = { BSP_USART0_PERIPH };

static uint32_t g_printf_periph = BSP_USART0_PERIPH;

/* ---------------- DMA 共享变量定义 ---------------- */
uint8_t           dma_rx_buffer[DMA_RX_BUF_SIZE];
volatile uint16_t dma_rx_len      = 0;
volatile uint8_t  dma_rx_complete = 0;
volatile uint8_t  dma_tx_complete = 0;

/* ================== 基础 USART 初始化 ================== */
void bsp_usart_init(usart_typedef_enum usart_num,
                    usartmode_typedef_enum usart_mode,
                    uint32_t baudrate)
{
    if (usart_num >= COM_COUNT) return;

    uint32_t periph    = USART_PERIPH[usart_num];
    uint32_t gpio_port = USART_GPIO_PORT[usart_num];
    uint32_t tx_pin    = USART_TX_PIN[usart_num];
    uint32_t rx_pin    = USART_RX_PIN[usart_num];

    rcu_periph_clock_enable(USART_GPIO_CLK[usart_num]);
    rcu_periph_clock_enable(USART_CLK[usart_num]);

    gpio_af_set(gpio_port, USART_AF[usart_num], tx_pin | rx_pin);

    gpio_mode_set(gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, tx_pin);
    gpio_output_options_set(gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, tx_pin);

    gpio_mode_set(gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, rx_pin);
    gpio_output_options_set(gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, rx_pin);

    usart_deinit(periph);
    usart_baudrate_set(periph, baudrate);
    usart_word_length_set(periph, USART_WL_8BIT);
    usart_stop_bit_set(periph, USART_STB_1BIT);
    usart_parity_config(periph, USART_PM_NONE);
    usart_hardware_flow_rts_config(periph, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(periph, USART_CTS_DISABLE);
    usart_receive_config(periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(periph, USART_TRANSMIT_ENABLE);
    usart_enable(periph);

    switch (usart_mode)
    {
        case BSP_USART_MODE_PRINTF:
            break;

        case BSP_USART_MODE_IRQ:
            usart_interrupt_enable(periph, USART_INT_RBNE);
            nvic_irq_enable(BSP_USART0_IRQn, 1U, 0U);
            break;

        case BSP_USART_MODE_DMA:
            usart_dma_receive_config(periph, USART_DENT_ENABLE);
            usart_dma_transmit_config(periph, USART_DENT_ENABLE);
            break;

        default:
            break;
    }

    g_printf_periph = periph;
}

/* ================== DMA 接收初始化：DMA1_CH2 + 循环模式 + IDLE ================== */
void bsp_usart_dma_rx_init(void)
{
    dma_single_data_parameter_struct dma_init_struct;

    rcu_periph_clock_enable(RCU_DMA1);

    dma_deinit(BSP_USART0_DMA, BSP_USART0_RX_DMA_CH);

    dma_single_data_para_struct_init(&dma_init_struct);
    dma_init_struct.direction           = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr        = (uint32_t)dma_rx_buffer;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number              = DMA_RX_BUF_SIZE;
    dma_init_struct.periph_addr         = (uint32_t)&USART_DATA(BSP_USART0_PERIPH);
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority            = DMA_PRIORITY_ULTRA_HIGH;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_ENABLE;

    dma_single_data_mode_init(BSP_USART0_DMA, BSP_USART0_RX_DMA_CH, &dma_init_struct);
    dma_channel_subperipheral_select(BSP_USART0_DMA, BSP_USART0_RX_DMA_CH,
                                     BSP_USART0_DMA_SUBPERI);

    dma_channel_enable(BSP_USART0_DMA, BSP_USART0_RX_DMA_CH);

    usart_dma_receive_config(BSP_USART0_PERIPH, USART_RECEIVE_DMA_ENABLE);
    usart_interrupt_enable(BSP_USART0_PERIPH, USART_INT_IDLE);
    nvic_irq_enable(BSP_USART0_IRQn, 1U, 0U);
}

/* ================== DMA 发送：DMA1_CH7 + 普通模式 + FTF 中断 ================== */
void bsp_usart_dma_tx_start(uint8_t *data, uint16_t len)
{
    dma_single_data_parameter_struct dma_init_struct;

    if (len == 0 || data == NULL) return;

    /* 若上一帧还没发完，先关通道，避免覆盖正在用的配置 */
    dma_channel_disable(BSP_USART0_DMA, BSP_USART0_TX_DMA_CH);

    dma_deinit(BSP_USART0_DMA, BSP_USART0_TX_DMA_CH);

    dma_single_data_para_struct_init(&dma_init_struct);
    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr        = (uint32_t)data;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.number              = len;
    dma_init_struct.periph_addr         = (uint32_t)&USART_DATA(BSP_USART0_PERIPH);
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority            = DMA_PRIORITY_ULTRA_HIGH;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;

    dma_single_data_mode_init(BSP_USART0_DMA, BSP_USART0_TX_DMA_CH, &dma_init_struct);
    dma_channel_subperipheral_select(BSP_USART0_DMA, BSP_USART0_TX_DMA_CH,
                                     BSP_USART0_DMA_SUBPERI);

    usart_dma_transmit_config(BSP_USART0_PERIPH, USART_TRANSMIT_DMA_ENABLE);

    dma_interrupt_enable(BSP_USART0_DMA, BSP_USART0_TX_DMA_CH, DMA_INT_FLAG_FTF);
    nvic_irq_enable(BSP_USART0_TX_DMA_IRQn, 2U, 0U);

    dma_channel_enable(BSP_USART0_DMA, BSP_USART0_TX_DMA_CH);
}

/* ================== printf 重定向 ================== */
#if defined(__GNUC__) && !defined(__clang__)
int __io_putchar(int ch)
{
    usart_data_transmit(g_printf_periph, (uint8_t)ch);
    while (RESET == usart_flag_get(g_printf_periph, USART_FLAG_TBE));
    return ch;
}
#else
int fputc(int ch, FILE *f)
{
    usart_data_transmit(g_printf_periph, (uint8_t)ch);
    while (RESET == usart_flag_get(g_printf_periph, USART_FLAG_TBE));
    return ch;
}
#endif