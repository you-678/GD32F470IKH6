#include "app_adc.h"
#include "bsp_adc.h"
#include "systick.h"
#include <stdio.h>

/*!
    \brief      ADC 内部通道采集任务
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_adc_task_internal(void) {
    static uint32_t tick = 0;
    tick++;
    
    /* main 循环延时 10ms，累加 200 次即为 2000ms */
    if (tick >= 200) {
        tick = 0;
        bsp_adc_trigger(); 
        delay_1ms(1); 
        
        float temp = (1.42f - bsp_adc_get_idata0() * 3.3f / 4096.0f) * 1000.0f / 4.35f + 25.0f;
        float vref = (bsp_adc_get_idata1() * 3.3f / 4096.0f);
        float vbat = (bsp_adc_get_idata2() * 4.0f * 3.3f / 4096.0f);
        
        printf("[INTERNAL] Temp: %2.0f C, Vref: %5.3f V, Vbat: %5.3f V\r\n", temp, vref, vbat);
    }
}

/*!
    \brief      ADC 双通道 DMA 采集任务
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_adc_task_dual_dma(void) {
    static uint32_t tick = 0;
    tick++;
    
    if (tick >= 200) { 
        tick = 0;
        printf("[DUAL DMA] Raw Data0: %08X, Data1: %08X\r\n", 
               bsp_adc_get_dma_data0(), bsp_adc_get_dma_data1());
    }
}