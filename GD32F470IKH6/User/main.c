#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
#include "drv_i2s_player.h"

int main(void)
{
    /* 系统底层初始化 */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    systick_config(); /* 呼吸灯依赖 systick 的延时 */
    
    /* 可以保留串口方便调试 */
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);

    /* App 初始化 */
    app_breath_init();

    /* 主循环 */
    while(1) {
        app_breath_task();
    }
}


//int main(void)
//{
//    /* 系统时基 + 中断优先级 */
//    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
//    systick_config();

//    /* BSP 初始化 */
//    bsp_led_init(LED1);
//    bsp_led_init(LED2);
//    bsp_led_init(LED3);
//    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

//    /* SD 卡中断使能（DMA 模式必须） */
//    nvic_irq_enable(SDIO_IRQn, 1U, 0U);

//    /* SD 卡应用流程 */
//    app_sdcard_init();
//    app_sdcard_config();
//    app_sdcard_info_print();
//    app_sdcard_test();

//    while(1) {
//        /* 主循环：可放 app_usart_task() 等 */
//    }
//}