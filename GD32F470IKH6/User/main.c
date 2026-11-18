#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
#include "drv_i2s_player.h"
#include "systick.h"
#include "app_scheduler.h"
#include "app_nb_led.h"
#include "app_nb_key.h"

int main(void)
{
    /* 串口：中断接收模式，115200 */
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);

    /* ===== 新增：非阻塞调度地基 ===== */
    systick_config();              /* 必须！没有它 uwTick 永远是 0 */
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);
    bsp_key_init(KEY_WAKEUP, KEY_MODE_GPIO);
    nb_led_start(500);             /* 流水灯：每盏亮 500ms */
    nb_key_start(5);               /* 按键：5ms 扫一次（放调度器最前，响应优先） */
    printf("\r\n /==============Gigadevice TRNG test=============/ \r\n");

    /* TRNG 初始化（内部已带重试） */
    if(ERROR == bsp_trng_init()) {
        printf("TRNG init error \r\n");
        while(1);
    }
    printf("TRNG init ok \r\n");
    printf("Send 'TRNG' to generate random number.\r\n");

    while(1) {
        /* 串口应用任务：命令解析、DMA 回显、LED 翻转都在里面 */
        task_scheduler();          /* 轮询所有定时类外设（LED/按键…） */
        app_usart_task();          /* 事件型：收到串口数据才处理，不进调度数组 */
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