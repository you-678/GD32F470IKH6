#include "app_adc.h"

/*!
    \brief      ADC 定时采集应用任务
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_adc_task(void) {
    static uint32_t tick = 0;
    
    delay_1ms(1); /* 充当系统心跳节拍 */
    tick++;
    
    if (tick >= 2000) {
        tick = 0;
        
        bsp_adc_trigger(); 
        delay_1ms(1); /* 等待 ADC 转换完成 */
        
        float temp = (1.42f - bsp_adc_get_idata0() * 3.3f / 4096.0f) * 1000.0f / 4.35f + 25.0f;
        float vref = (bsp_adc_get_idata1() * 3.3f / 4096.0f);
        float vbat = (bsp_adc_get_idata2() * 4.0f * 3.3f / 4096.0f);
        
        printf("Temp: %2.0f C, Vref: %5.3f V, Vbat: %5.3f V\r\n", temp, vref, vbat);
    }
}