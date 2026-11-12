#ifndef __BSP_DAC_H
#define __BSP_DAC_H

#include "gd32f4xx.h"

/*!
    \brief      初始化 DAC 硬件
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void bsp_dac_init(void);

/*!
    \brief      设置 DAC 输出电压
    \param[in]  value: 12位数字量 (0~4095)。0对应0V，4095对应VDDA(通常为3.3V)
    \param[out] 无
    \retval     无
*/
void bsp_dac_set_value(uint16_t value);

#endif