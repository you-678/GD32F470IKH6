#ifndef __APP_DAC_H
#define __APP_DAC_H

#include "gd32f4xx.h"

/*!
    \brief      DAC 阶梯波扫描任务 (非阻塞，需在主循环不断调用)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_dac_scan_task(void);

#endif