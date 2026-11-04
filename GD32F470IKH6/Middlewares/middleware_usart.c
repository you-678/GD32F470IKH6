#include "middleware_usart.h"

// printf重定向
int fputc(int ch, FILE *f)
{
    bsp_usart_send_byte((uint8_t)ch);
    return ch;
}

void middleware_usart_init(uint32_t baudrate)
{
    bsp_usart_init(baudrate);
}

void middleware_usart_send_string(char *str)
{
    while(*str != '\0')
    {
        bsp_usart_send_byte((uint8_t)*str++);
    }
    //等待发送完成 TC
    while(RESET == usart_flag_get(BSP_USART_COM0, USART_FLAG_TC));
}
