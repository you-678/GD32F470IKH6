#include "bsp_usart.h"

static rcu_periph_enum COM_CLK[COMn] = {EVAL_COM0_CLK};
static uint32_t COM_TX_PIN[COMn] = {EVAL_COM0_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {EVAL_COM0_RX_PIN};

void bsp_usart_init(uint32_t com, uint32_t baudrate)
{
    /* enable GPIO clock */
    uint32_t COM_ID = 0;
    if(EVAL_COM0 == com)
    {
        COM_ID = 0U;
    }

    rcu_periph_clock_enable( EVAL_COM0_GPIO_CLK);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[COM_ID]);

    /* connect port to USARTx_Tx */
    gpio_af_set(EVAL_COM0_GPIO_PORT, EVAL_COM0_AF, COM_TX_PIN[COM_ID]);

    /* connect port to USARTx_Rx */
    gpio_af_set(EVAL_COM0_GPIO_PORT, EVAL_COM0_AF, COM_RX_PIN[COM_ID]);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(EVAL_COM0_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP,COM_TX_PIN[COM_ID]);
    gpio_output_options_set(EVAL_COM0_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,COM_TX_PIN[COM_ID]);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(EVAL_COM0_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP,COM_RX_PIN[COM_ID]);
    gpio_output_options_set(EVAL_COM0_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,COM_RX_PIN[COM_ID]);

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com,baudrate);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}
#if defined(__GNUC__) && !defined(__clang__) /* For GNU GCC compiler */
/* retarget the C library printf function to the USART, in GCC environment */
int __io_putchar(int ch)
{
    usart_data_transmit(EVAL_COM0, (uint8_t) ch);
    while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));
    return ch;
}
#else
/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM0, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE));

    return ch;
}
#endif
//uint8_t bsp_usart_recv_byte(void)
//{
//    while(RESET == usart_flag_get(BSP_USART_COM0, USART_FLAG_RBNE));
//    return usart_data_receive(BSP_USART_COM0);
//}
