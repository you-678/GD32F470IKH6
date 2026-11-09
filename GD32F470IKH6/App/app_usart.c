#include "app_usart.h"

void app_usart_task(void)
{
    if(RESET == bsp_key_state_get(KEY_TAMPER))
    {
        delay_1ms(50);
        if(RESET == bsp_key_state_get(KEY_TAMPER))
        {
            delay_1ms(50);
            if(RESET == bsp_key_state_get(KEY_TAMPER))
            {
                bsp_led_on(LED3);
                printf("\r\n USART printf example \r\n");
            }
            else
            {
                bsp_led_off(LED3);
            }
        }
        else
        {
            bsp_led_off(LED3);
        }
    }
    else
    {
        bsp_led_off(LED3);
    }
}
