#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "gd32f4xx.h"

/* 定义 ADC 工作模式 */
typedef enum {
    BSP_ADC_MODE_INTERNAL = 0,  /* 内部通道 (温度、Vref、VBAT)，软件触发 */
    BSP_ADC_MODE_DUAL_DMA       /* 外部双ADC同步 (PC3/PC5)，定时器触发+DMA */
} bsp_adc_mode_enum;

/* 对外统一接口 */
void bsp_adc_init(bsp_adc_mode_enum mode);
void bsp_adc_trigger(void);

/* 内部通道数据获取 */
uint16_t bsp_adc_get_idata0(void);
uint16_t bsp_adc_get_idata1(void);
uint16_t bsp_adc_get_idata2(void);

/* 外部 DMA 通道数据获取 */
uint32_t bsp_adc_get_dma_data0(void);
uint32_t bsp_adc_get_dma_data1(void);

#endif