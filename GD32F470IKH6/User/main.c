#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

/*!
    \brief      系统主函数 - ADC 三种模式轮番测试
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    /* =========================================================
     * 阶段一：测试 内部通道 软件触发模式 (持续 10 秒)
     * ========================================================= */
    bsp_adc_init(BSP_ADC_MODE_INTERNAL);
    printf("\r\n========================================\r\n");
    printf("STAGE 1: INTERNAL ADC (10s)\r\n");
    printf("========================================\r\n");
    
    for (int i = 0; i < 1000; i++) { /* 1000 * 10ms = 10s */
        app_adc_task_internal();
        delay_1ms(10);
    }

    /* =========================================================
     * 阶段二：测试 双ADC 跟随模式 DMA (持续 10 秒)
     * ========================================================= */
    bsp_adc_init(BSP_ADC_MODE_DUAL_DMA_FOLLOW_UP);
    printf("\r\n========================================\r\n");
    printf("STAGE 2: DUAL ADC FOLLOW UP MODE (10s)\r\n");
    printf("========================================\r\n");
    
    for (int i = 0; i < 1000; i++) {
        app_adc_task_dual_dma();
        delay_1ms(10);
    }

    /* =========================================================
     * 阶段三：测试 双ADC 并行模式 DMA (持续 10 秒)
     * ========================================================= */
    bsp_adc_init(BSP_ADC_MODE_DUAL_DMA_PARALLEL);
    printf("\r\n========================================\r\n");
    printf("STAGE 3: DUAL ADC PARALLEL MODE (10s)\r\n");
    printf("========================================\r\n");
    
    for (int i = 0; i < 1000; i++) {
        app_adc_task_dual_dma();
        delay_1ms(10);
    }

    printf("\r\nAll ADC Modes Tested OK!\r\n");

    while(1) {
        delay_1ms(1000); /* 测试结束，停在此处 */
    }
}