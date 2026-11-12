#include "bsp_adc.h"

static __IO uint32_t adc_dma_value[2];
static bsp_adc_mode_enum g_adc_mode = BSP_ADC_MODE_INTERNAL;

/*!
    \brief      配置内部通道 ADC (软件触发)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
static void adc_internal_config(void) {
    rcu_periph_clock_enable(RCU_ADC0);
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
    adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 3);
    adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_16, ADC_SAMPLETIME_480);
    adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_17, ADC_SAMPLETIME_480);
    adc_inserted_channel_config(ADC0, 2, ADC_CHANNEL_18, ADC_SAMPLETIME_480);
    adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, DISABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_channel_16_to_18(ADC_VBAT_CHANNEL_SWITCH, ENABLE);
    adc_channel_16_to_18(ADC_TEMP_VREF_CHANNEL_SWITCH, ENABLE);
    adc_enable(ADC0);
    adc_calibration_enable(ADC0);
}

/*!
    \brief      配置双ADC硬件触发用的定时器 (TIMER1)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
static void adc_dual_dma_timer_config(void) {
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler = 19999U;
    timer_initpara.period = 9999U;
    timer_init(TIMER1, &timer_initpara);
    timer_channel_output_struct_para_init(&timer_ocintpara);
    timer_ocintpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
    timer_channel_output_config(TIMER1, TIMER_CH_1, &timer_ocintpara);
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, 3999U);
    timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);
    timer_enable(TIMER1);
}

/*!
    \brief      配置双ADC同步模式及DMA搬运
    \param[in]  mode: 工作模式(跟随或并行)
    \param[out] 无
    \retval     无
*/
static void adc_dual_dma_config(bsp_adc_mode_enum mode) {
    rcu_periph_clock_enable(BSP_ADC_DUAL_GPIO_CLK);
    rcu_periph_clock_enable(RCU_DMA1);
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_ADC1);
    
    /* 根据官方Demo，并行模式ADC时钟为DIV4，跟随模式为DIV8 */
    if (mode == BSP_ADC_MODE_DUAL_DMA_PARALLEL) {
        adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
    } else {
        adc_clock_config(ADC_ADCCK_PCLK2_DIV8);
    }

    gpio_mode_set(BSP_ADC_DUAL_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_ADC_CH13_PIN);
    gpio_mode_set(BSP_ADC_DUAL_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_ADC_CH15_PIN);
    adc_dual_dma_timer_config();

    dma_single_data_parameter_struct dma_para;
    dma_single_data_para_struct_init(&dma_para);
    dma_deinit(DMA1, DMA_CH0);
    dma_para.periph_addr = (uint32_t)(&ADC_SYNCDATA);
    dma_para.memory0_addr = (uint32_t)(adc_dma_value);
    dma_para.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_para.periph_memory_width = DMA_PERIPH_WIDTH_32BIT;
    dma_para.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    dma_para.direction = DMA_PERIPH_TO_MEMORY;
    dma_para.number = 2;
    dma_para.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_para);
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    dma_channel_enable(DMA1, DMA_CH0);

       if (mode == BSP_ADC_MODE_DUAL_DMA_PARALLEL) {
        /* 并行模式 */
        adc_sync_mode_config(ADC_DAUL_REGULAL_PARALLEL);
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_13, ADC_SAMPLETIME_144);
        adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_15, ADC_SAMPLETIME_144);
        adc_regular_channel_config(ADC1, 1, ADC_CHANNEL_13, ADC_SAMPLETIME_144);
        adc_regular_channel_config(ADC1, 0, ADC_CHANNEL_15, ADC_SAMPLETIME_144);
    } else {
        /* 跟随模式 */
        adc_sync_mode_config(ADC_DAUL_REGULAL_FOLLOW_UP);
        adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_13, ADC_SAMPLETIME_480);
        adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_15, ADC_SAMPLETIME_480);
        adc_regular_channel_config(ADC1, 1, ADC_CHANNEL_13, ADC_SAMPLETIME_480);
        adc_regular_channel_config(ADC1, 0, ADC_CHANNEL_15, ADC_SAMPLETIME_480);
    }

    adc_sync_dma_config(ADC_SYNC_DMA_MODE1);
    adc_sync_dma_request_after_last_enable();
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_special_function_config(ADC1, ADC_SCAN_MODE, ENABLE);
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 2);
    adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 2);
    
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_RISING);
    adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T1_CH1);
    
    adc_enable(ADC0); adc_calibration_enable(ADC0);
    adc_enable(ADC1); adc_calibration_enable(ADC1);
}

/*!
    \brief      初始化 ADC 硬件
    \param[in]  mode: 选择 ADC 工作模式
    \param[out] 无
    \retval     无
*/
void bsp_adc_init(bsp_adc_mode_enum mode) {
    /* 切换模式前，先强制关闭所有相关外设，防止硬件死锁 */
    dma_channel_disable(DMA1, DMA_CH0);
    timer_disable(TIMER1);
    adc_disable(ADC0);
    adc_disable(ADC1);

    g_adc_mode = mode;
    
    if (mode == BSP_ADC_MODE_INTERNAL) {
        adc_internal_config();
    } else {
        adc_dual_dma_config(mode);
    }
}

/*!
    \brief      通过软件手动触发一次内部 ADC 转换
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void bsp_adc_trigger(void) { 
    if (g_adc_mode == BSP_ADC_MODE_INTERNAL) adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL); 
}

uint16_t bsp_adc_get_idata0(void) { return ADC_IDATA0(ADC0); }
uint16_t bsp_adc_get_idata1(void) { return ADC_IDATA1(ADC0); }
uint16_t bsp_adc_get_idata2(void) { return ADC_IDATA2(ADC0); }
uint32_t bsp_adc_get_dma_data0(void) { return adc_dma_value[0]; }
uint32_t bsp_adc_get_dma_data1(void) { return adc_dma_value[1]; }