#include "app_usart_irq.h"

//void app_usart_irq_init(void)
//{
//	middleware_usart_reset();
//	app_usart_init();
//	bsp_usart_irq_enable();
//}

//void app_usart_irq_task(void)
//{
//    /* 等待发送全部完成 */
//    while(tx_counter < nbr_data_to_send);
//    /* 等待接收全部完成 */
//    while(rx_counter < nbr_data_to_read);

//    /* 数据对比校验 */
//    transfer_status = middleware_usart_compare(tx_buffer, rx_buffer, BUFFER_SIZE);

//    while(1)
//    {
//        if(SUCCESS == transfer_status)
//        {
//            bsp_led_on(LED1);
//            delay_1ms(200);
//            bsp_led_on(LED2);
//            delay_1ms(200);
//            bsp_led_on(LED3);
//            delay_1ms(200);
//            bsp_led_off(LED1);
//            bsp_led_off(LED2);
//            bsp_led_off(LED3);
//			
//            delay_1ms(200);
//        }
//        else
//        {
//            app_led_flash(1);
//        }
//    }
//}