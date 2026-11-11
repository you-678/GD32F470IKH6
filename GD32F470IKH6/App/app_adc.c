#include "app_adc.h"

/* 在业务层宏定义一个默认模式，方便随时切换测试 */
#define APP_CURRENT_ADC_MODE    BSP_ADC_MODE_INTERNAL 
//#define APP_CURRENT_ADC_MODE    BSP_ADC_MODE_DUAL_DMA

void app_adc_task(void)
{
    static uint32_t tick_count = 0;

    /* 1. 系统心跳节拍 (每次进入任务延时 1ms) */
    delay_1ms(1); 
    tick_count++;

    /* 2. 每 2000ms 执行一次打印任务 */
    if (tick_count >= 2000) {
        tick_count = 0; 

        if (APP_CURRENT_ADC_MODE == BSP_ADC_MODE_INTERNAL) {
            /* ============ 内部通道采样 ============ */
            bsp_adc_trigger();  // 软件触发
            delay_1ms(1);       // 等待转换完成
            
            float temperature   = (1.42f - bsp_adc_get_idata0() * 3.3f / 4096.0f) * 1000.0f / 4.35f + 25.0f;
            float vref_value    = (bsp_adc_get_idata1() * 3.3f / 4096.0f);
            float battery_value = (bsp_adc_get_idata2() * 4.0f * 3.3f / 4096.0f);

            printf(" the temperature data is %2.0f degrees Celsius\r\n", temperature);
            printf(" the reference voltage data is %5.3fV \r\n", vref_value);
            printf(" the battery voltage is %5.3fV \r\n", battery_value);
        } 
        else {
            /* ============ 外部双通道 DMA 采样 ============ */
            /* 硬件已经在后台完成采集，直接读内存即可 */
            printf(" the data adc_value[0] is %08X \r\n", bsp_adc_get_dma_data0());
            printf(" the data adc_value[1] is %08X \r\n", bsp_adc_get_dma_data1());
        }
        printf(" \r\n");
    }
}