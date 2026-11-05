#include "app_usart.h"

void app_usart_init(void)
{
    middleware_usart_init(115200);
    printf("\r\n USART printf example: please press the Tamper key \r\n");
}

void app_usart_task(void)
{
    if(RESET == bsp_key_tamper_read())
    {
        delay_1ms(50);
        if(RESET == bsp_key_tamper_read())
        {
            delay_1ms(50);
            if(RESET == bsp_key_tamper_read())
            {
                bsp_led3_on();
                printf("\r\n USART printf example \r\n");
            }
            else
            {
                bsp_led3_off();
            }
        }
        else
        {
            bsp_led3_off();
        }
    }
    else
    {
        bsp_led3_off();
    }
}
