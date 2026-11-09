#include "bsp_usart.h"

/* 查表：按 usart_num 索引，新增 COM 只改表 + 头文件宏即可 */
#define COM_COUNT   (sizeof(USART_CLK) / sizeof(USART_CLK[0]))

static const rcu_periph_enum USART_CLK[]      = { BSP_USART0_CLK };
static const rcu_periph_enum USART_GPIO_CLK[] = { BSP_USART0_GPIO_CLK };
static const uint32_t       USART_GPIO_PORT[] = { BSP_USART0_GPIO_PORT };
static const uint32_t       USART_TX_PIN[]    = { BSP_USART0_TX_PIN };
static const uint32_t       USART_RX_PIN[]    = { BSP_USART0_RX_PIN };
static const uint32_t       USART_AF[]        = { BSP_USART0_AF };
static const uint32_t       USART_PERIPH[]    = { BSP_USART0_PERIPH };

/* printf 重定向使用的当前外设 */
static uint32_t g_printf_periph = BSP_USART0_PERIPH;

void bsp_usart_init(usart_typedef_enum usart_num,
                    usartmode_typedef_enum usart_mode,
                    uint32_t baudrate)
{
    /* ---- 参数检查 ---- */
    if (usart_num >= COM_COUNT)
    {
        return;          /* 不支持的 COM，直接返回 */
    }

    uint32_t periph    = USART_PERIPH[usart_num];
    uint32_t gpio_port = USART_GPIO_PORT[usart_num];
    uint32_t tx_pin    = USART_TX_PIN[usart_num];
    uint32_t rx_pin    = USART_RX_PIN[usart_num];

    /* ---- 1. 时钟 ---- */
    rcu_periph_clock_enable(USART_GPIO_CLK[usart_num]);
    rcu_periph_clock_enable(USART_CLK[usart_num]);

    /* ---- 2. GPIO 复用 ---- */
    gpio_af_set(gpio_port, USART_AF[usart_num], tx_pin | rx_pin);

    gpio_mode_set(gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, tx_pin);
    gpio_output_options_set(gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, tx_pin);

    gpio_mode_set(gpio_port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, rx_pin);
    gpio_output_options_set(gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, rx_pin);

    /* ---- 3. USART 基础配置（8N1）---- */
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

    /* ---- 4. 按模式追加配置 ---- */
    switch (usart_mode)
    {
        case BSP_USART_MODE_PRINTF:
            /* 轮询模式：printf 重定向即可，无需额外配置 */
            break;

        case BSP_USART_MODE_IRQ:
            /* 中断模式：先只开 RBNE，TBE 由发送方按需开 */
            usart_interrupt_enable(periph, USART_INT_RBNE);
            nvic_irq_enable(BSP_USART0_IRQn, 1U, 0U);
            break;

        case BSP_USART_MODE_DMA:
            /* DMA 模式：使能 USART 的 DMA 收发请求，DMA 通道由 bsp_usart_dma.c 配 */
            usart_dma_receive_config(periph, USART_DENR_ENABLE);
            usart_dma_transmit_config(periph, USART_DENT_ENABLE);
            break;

        default:
            break;
    }

    /* ---- 5. 记录 printf 目标外设 ---- */
    g_printf_periph = periph;
}

/* ---------------- printf 重定向 ---------------- */
#if defined(__GNUC__) && !defined(__clang__)
/* GNU GCC（如 arm-none-eabi-gcc）走 newlib 的 __io_putchar */
int __io_putchar(int ch)
{
    usart_data_transmit(g_printf_periph, (uint8_t)ch);
    while (RESET == usart_flag_get(g_printf_periph, USART_FLAG_TBE));
    return ch;
}
#else
/* Keil ARMCLANG / ARMCC 走 fputc */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(g_printf_periph, (uint8_t)ch);
    while (RESET == usart_flag_get(g_printf_periph, USART_FLAG_TBE));
    return ch;
}
#endif

///* 阻塞发送一个字节（轮询模式 / 调试用） */
//void bsp_usart_send_byte(usart_typedef_enum usart_num, uint8_t byte)
//{
//    if (usart_num >= COM_COUNT) return;

//    uint32_t periph = USART_PERIPH[usart_num];
//    usart_data_transmit(periph, byte);
//    while (RESET == usart_flag_get(periph, USART_FLAG_TBE));
//}

///* 阻塞接收一个字节（轮询模式 / 调试用） */
//uint8_t bsp_usart_recv_byte(usart_typedef_enum usart_num)
//{
//    if (usart_num >= COM_COUNT) return 0;

//    uint32_t periph = USART_PERIPH[usart_num];
//    while (RESET == usart_flag_get(periph, USART_FLAG_RBNE));
//    return (uint8_t)usart_data_receive(periph);
//}