#ifndef __APP_ADC_H
#define __APP_ADC_H

#include "gd32f4xx.h"

/*!
    \brief      ADC 定时采集应用任务 (内部通道，每2秒打印一次，非阻塞)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_adc_task(void);

#endif