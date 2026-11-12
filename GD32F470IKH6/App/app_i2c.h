#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "gd32f4xx.h"

/*!
    \brief      ADC 工作模式枚举
      \arg        BSP_ADC_MODE_INTERNAL: 内部通道模式 (采集温度/Vref/Vbat)，软件触发
      \arg        BSP_ADC_MODE_DUAL_DMA_FOLLOW_UP: 双ADC跟随模式 (采集CH13/CH15)，硬件定时器触发
      \arg        BSP_ADC_MODE_DUAL_DMA_PARALLEL: 双ADC并行模式 (采集CH13/CH15)，硬件定时器触发
*/
typedef enum { 
    BSP_ADC_MODE_INTERNAL = 0, 
    BSP_ADC_MODE_DUAL_DMA_FOLLOW_UP,
    BSP_ADC_MODE_DUAL_DMA_PARALLEL
} bsp_adc_mode_enum;

/*!
    \brief      初始化 ADC 硬件
    \param[in]  mode: 选择 ADC 工作模式
      \arg        BSP_ADC_MODE_INTERNAL: 内部通道软件触发
      \arg        BSP_ADC_MODE_DUAL_DMA_FOLLOW_UP: 双通道DMA硬件触发 (跟随)
      \arg        BSP_ADC_MODE_DUAL_DMA_PARALLEL: 双通道DMA硬件触发 (并行)
    \param[out] 无
    \retval     无
*/
void bsp_adc_init(bsp_adc_mode_enum mode);

/*!
    \brief      通过软件手动触发一次内部 ADC 转换
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void bsp_adc_trigger(void);

/*!
    \brief      获取内部 ADC 通道 0 的数据 (内部温度传感器)
    \param[in]  无
    \param[out] 无
    \retval     对应的原始 ADC 采样值
*/
uint16_t bsp_adc_get_idata0(void);

/*!
    \brief      获取内部 ADC 通道 1 的数据 (内部参考电压)
    \param[in]  无
    \param[out] 无
    \retval     对应的原始 ADC 采样值
*/
uint16_t bsp_adc_get_idata1(void);

/*!
    \brief      获取内部 ADC 通道 2 的数据 (内部电池电压)
    \param[in]  无
    \param[out] 无
    \retval     对应的原始 ADC 采样值
*/
uint16_t bsp_adc_get_idata2(void);

/*!
    \brief      获取外部双通道 DMA 模式搬运过来的数据 0
    \param[in]  无
    \param[out] 无
    \retval     对应的 DMA 搬运过来的原始数据
*/
uint32_t bsp_adc_get_dma_data0(void);

/*!
    \brief      获取外部双通道 DMA 模式搬运过来的数据 1
    \param[in]  无
    \param[out] 无
    \retval     对应的 DMA 搬运过来的原始数据
*/
uint32_t bsp_adc_get_dma_data1(void);

#endif