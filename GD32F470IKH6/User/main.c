/********************************************************************************
  * 文 件 名: main.c
  * 版 本 号: 初版
  * 修改作者: LC
  * 修改日期: 2022年04月12日
  * 功能介绍:          
  ******************************************************************************
  * 注意事项:
*********************************************************************************/

#include "gd32f4xx.h"
//#include "gd32f470i_eval.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"

#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_key_exti.h"
#include "bsp_usart.h"

#include "app_led.h"
#include "app_key.h"
#include "app_usart.h"

											 
/************************************************
函数名称 ： main
功    能 ： 主函数
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
int main(void)
{
	systick_config();
  bsp_led_init();
	bsp_key_init();
	bsp_key_exti_init();
	app_usart_init();

	app_led_flash(2);  // 上电闪烁一次自检
	
  while(1)
	{
//		led_flow_task();
//		key_scan_task();
		
		app_usart_task();
	}
}
