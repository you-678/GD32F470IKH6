#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

/*!
    \brief      系统主函数 - DAC 阶梯波扫描测试
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    /* 1. 初始化基础环境 */
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("DAC PA4 Scan Test Started!\r\n");
    printf("Outputting 0V -> 3.3V -> 0V continuously.\r\n");
    printf("Measure PA4 with a multimeter to see the change.\r\n");
    printf("========================================\r\n");

    /* 2. 初始化 DAC 硬件 */
    bsp_dac_init();

    /* 3. 主循环 */
    while(1) {
        /* 调用业务层任务，不断改变电压 */
        app_dac_scan_task();
        
        /* 延时 10ms 作为系统心跳 */
        delay_1ms(10);
    }
}