#include "gd32f4xx.h"
#include "main.h"
//#include "gd32f470i_eval.h"

int main(void)
{
	systick_config();
	bsp_led_init();
	bsp_key_init();
//	bsp_key_exti_init();
	
//	app_usart_irq_init();	
//	app_usart_irq_task();
	
	app_usart_dma_init();
	
	//app_led_flash(2);  // 上电闪烁一次自检
	
	while(1)
	{		
		//led_flow_task();
		//key_scan_task();
		//app_usart_task();
		app_usart_dma_loop(); //还没调通
	}
}
