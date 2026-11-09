#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

int main(void)
{
    systick_config();
    bsp_led_init(LED1);bsp_led_init(LED2);bsp_led_init(LED3);
    bsp_key_init(KEY_WAKEUP, KEY_MODE_GPIO);bsp_key_init(KEY_TAMPER, KEY_MODE_GPIO);bsp_key_init(KEY_USER, KEY_MODE_GPIO);
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
	
	app_led_flash(2);   //上电LED闪烁两次，和原demo保持一致
	
    while(1)
    {
        app_usart_task();
    }
}

//extern uint8_t           dma_rx_buffer[DMA_RX_BUF_SIZE];
//extern volatile uint16_t dma_rx_len;
//extern volatile uint8_t  dma_rx_complete;
//extern volatile uint8_t  dma_tx_complete;

//int main(void)
//{
//    systick_config();

//    bsp_led_init(LED1);
//    bsp_led_init(LED2);
//    bsp_led_init(LED3);

//    /* ① 配 USART 基础参数 + 使能 USART 的 DMA 收发请求 */
//    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_DMA, 115200U);

//    /* ② 配 DMA 通道（RX=DMA1_CH2，循环模式）+ IDLE 中断 */
//    bsp_usart_dma_rx_init();

//    app_led_flash(2);   /* 上电自检 */

//    while (1)
//    {
//        /* ---------------- 接收完成：把收到的内容用 DMA 回显 ---------------- */
//        if (dma_rx_complete)
//        {
//            dma_rx_complete = 0;

//            uint16_t len = dma_rx_len;
//            if (len > DMA_RX_BUF_SIZE) len = DMA_RX_BUF_SIZE;
//            if (len < DMA_RX_BUF_SIZE) dma_rx_buffer[len] = '\0';

//            bsp_usart_dma_tx_start(dma_rx_buffer, len);
//            bsp_led_toggle(LED1);     /* 每收到一帧，LED1 翻一次 */
//        }

//        /* ---------------- 发送完成：翻 LED2 ---------------- */
//        if (dma_tx_complete)
//        {
//            dma_tx_complete = 0;
//            bsp_led_toggle(LED2);     /* 每发完一帧，LED2 翻一次 */
//        }
//    }
//}


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
