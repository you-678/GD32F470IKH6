#include "bsp_usart.h"

void bsp_usart_init(uint32_t baudrate)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    gpio_af_set(BSP_USART_GPIO_PORT, GPIO_AF_7, BSP_USART_TX_PIN);
    gpio_af_set(BSP_USART_GPIO_PORT, GPIO_AF_7, BSP_USART_RX_PIN);
    gpio_mode_set(BSP_USART_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_USART_TX_PIN|BSP_USART_RX_PIN);
    gpio_output_options_set(BSP_USART_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_TX_PIN|BSP_USART_RX_PIN);

    usart_deinit(BSP_USART_COM0);
    usart_baudrate_set(BSP_USART_COM0, baudrate);
    usart_transmit_config(BSP_USART_COM0, USART_TRANSMIT_ENABLE);
    usart_receive_config(BSP_USART_COM0, USART_RECEIVE_ENABLE);
    usart_enable(BSP_USART_COM0);
}


void bsp_usart_send_byte(uint8_t dat)
{
    usart_data_transmit(BSP_USART_COM0, dat);
    while(RESET == usart_flag_get(BSP_USART_COM0, USART_FLAG_TBE));
}

uint8_t bsp_usart_recv_byte(void)
{
    while(RESET == usart_flag_get(BSP_USART_COM0, USART_FLAG_RBNE));
    return usart_data_receive(BSP_USART_COM0);
}
