#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

/* 全局接收缓冲区和标志位 */
extern uint8_t  rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_index;
extern volatile uint8_t  rx_complete_flag;

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);  // 中断模式

    while (1)
    {
        if (rx_complete_flag)
        {
            /* 处理收到的完整一帧数据 */
            // 例如：回显收到的字符串
            printf("IRQ Recv: %s", rx_buffer);
            
            /* 清空状态，准备接收下一帧 */
            memset(rx_buffer, 0, sizeof(rx_buffer));
            rx_index = 0;
            rx_complete_flag = 0;
        }
        
        /* 可以在这里执行其他非阻塞任务 */
    }
}

//int main(void)
//{
//	systick_config();
//	bsp_led_init();
//	bsp_key_init();
////	bsp_key_exti_init();
//	
////	app_usart_irq_init();	
////	app_usart_irq_task();
//	
//	app_usart_dma_init();
//	
//	//app_led_flash(2);  // 上电闪烁一次自检
//	
//	while(1)
//	{		
//		//led_flow_task();
//		//key_scan_task();
//		//app_usart_task();
//		app_usart_dma_loop(); //还没调通
//	}
//}
