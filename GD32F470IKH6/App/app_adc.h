#ifndef __APP_ADC_H
#define __APP_ADC_H

#include "gd32f4xx.h"

/*!
    \brief      ADC 内部通道采集任务 (软件触发，每2秒打印)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_adc_task_internal(void);

/*!
    \brief      ADC 双通道 DMA 采集任务 (硬件触发，每2秒打印)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_adc_task_dual_dma(void);

#endif