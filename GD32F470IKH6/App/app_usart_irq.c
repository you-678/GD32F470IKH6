#include "app_usart.h"
#include "app_led.h"

void app_usart_init(void)
{
	middleware_usart_reset();
    /* 初始化串口中间件 115200 */
    middleware_usart_init(115200);
    /* 按键、LED初始化 */
//    bsp_key_tamper_init();
//    bsp_led_init();
    /* 上电提示打印 */
    printf("\r\n USART printf example: please press the Tamper key \r\n");
}

//void app_usart_task(void)
//{
//    if(RESET == bsp_key_tamper_read())
//    {
//        delay_1ms(50);
//        if(RESET == bsp_key_tamper_read())
//        {
//            delay_1ms(50);
//            if(RESET == bsp_key_tamper_read())
//            {
//                bsp_led3_on();
//                printf("\r\n USART printf example \r\n");
//            }
//            else
//            {
//                bsp_led3_off();
//            }
//        }
//        else
//        {
//            bsp_led3_off();
//        }
//    }
//    else
//    {
//        bsp_led3_off();
//    }
//}
void app_usart_task(void)
{
    /* 等待发送全部完成 */
    while(tx_counter < nbr_data_to_send);
    /* 等待接收全部完成 */
    while(rx_counter < nbr_data_to_read);

    /* 数据对比校验 */
    transfer_status = middleware_usart_compare(tx_buffer, rx_buffer, BUFFER_SIZE);

    while(1)
    {
        if(SUCCESS == transfer_status)
        {
            bsp_led1_on();
            delay_1ms(200);
            bsp_led2_on();
            delay_1ms(200);
            bsp_led3_on();
            delay_1ms(200);
            bsp_led1_off();
            bsp_led2_off();
            bsp_led3_off();
            delay_1ms(200);
        }
        else
        {
            app_led_flash(1);
        }
    }
}