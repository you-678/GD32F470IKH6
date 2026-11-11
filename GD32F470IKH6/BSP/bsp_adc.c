#include "bsp_adc.h"

/* ================== 硬件定义与映射数组 ================== */
#define BSP_ADC_DUAL_GPIO_CLK     RCU_GPIOC
#define BSP_ADC_DUAL_GPIO_PORT    GPIOC
#define BSP_ADC_CH13_PIN          GPIO_PIN_3
#define BSP_ADC_CH15_PIN          GPIO_PIN_5

static const rcu_periph_enum ADC_GPIO_CLK[]  = { BSP_ADC_DUAL_GPIO_CLK };
static const uint32_t        ADC_GPIO_PORT[] = { BSP_ADC_DUAL_GPIO_PORT };
static const uint32_t        ADC_PIN_13[]    = { BSP_ADC_CH13_PIN };
static const uint32_t        ADC_PIN_15[]    = { BSP_ADC_CH15_PIN };

/* DMA 搬运的目标缓冲区 (供双通道模式使用) */
static __IO uint32_t adc_dma_value[2];

/* 记录当前 ADC 工作模式，供 trigger 函数判断 */
static bsp_adc_mode_enum g_adc_mode = BSP_ADC_MODE_INTERNAL;

/* ================== 内部私有配置函数 (static) ================== */

/* 模式 1：内部通道配置 (对应 Demo 07) */
static void adc_internal_config(void)
{
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

/* 模式 2 辅助：定时器配置 (用于硬件触发双 ADC) */
static void adc_dual_dma_timer_config(void)
{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    
    rcu_periph_clock_enable(RCU_TIMER1);
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    
    timer_struct_para_init(&timer_initpara);
    timer_channel_output_struct_para_init(&timer_ocintpara);
    
    timer_initpara.prescaler         = 19999U;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 9999U;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0U;
    timer_init(TIMER1, &timer_initpara);

    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
    timer_ocintpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1, TIMER_CH_1, &timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, 3999U);
    timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);
    
    timer_enable(TIMER1);
}

/* 模式 2：双 ADC 同步 DMA 配置 (对应 Demo 08 / 09) */
static void adc_dual_dma_config(void)
{
    /* 1. 时钟与引脚 */
    rcu_periph_clock_enable(ADC_GPIO_CLK[0]);
    rcu_periph_clock_enable(RCU_DMA1);
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_ADC1);
    adc_clock_config(ADC_ADCCK_PCLK2_DIV8);

    gpio_mode_set(ADC_GPIO_PORT[0], GPIO_MODE_ANALOG, GPIO_PUPD_NONE, ADC_PIN_13[0]);
    gpio_mode_set(ADC_GPIO_PORT[0], GPIO_MODE_ANALOG, GPIO_PUPD_NONE, ADC_PIN_15[0]);

    /* 2. 硬件触发定时器 */
    adc_dual_dma_timer_config();

    /* 3. DMA 配置 */
    dma_single_data_parameter_struct dma_single_data_parameter;
    dma_single_data_para_struct_init(&dma_single_data_parameter);
    dma_deinit(DMA1, DMA_CH0);
    
    dma_single_data_parameter.periph_addr = (uint32_t)(&ADC_SYNCDATA);
    dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_single_data_parameter.memory0_addr = (uint32_t)(adc_dma_value);
    dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_32BIT;
    dma_single_data_parameter.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;
    dma_single_data_parameter.number = 2;
    dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_single_data_parameter);
    
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);
    dma_channel_enable(DMA1, DMA_CH0);

    /* 4. ADC 同步模式配置 */
    adc_sync_mode_config(ADC_DAUL_REGULAL_FOLLOW_UP);
	adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 2);
	adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 2);
	adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_13, ADC_SAMPLETIME_480);
	adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_15, ADC_SAMPLETIME_480);
	adc_regular_channel_config(ADC1, 1, ADC_CHANNEL_13, ADC_SAMPLETIME_480);
	adc_regular_channel_config(ADC1, 0, ADC_CHANNEL_15, ADC_SAMPLETIME_480);
	adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_RISING);
	adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
	adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T1_CH1);
    
    adc_enable(ADC0);
    adc_calibration_enable(ADC0);
    adc_enable(ADC1);
    adc_calibration_enable(ADC1);
}

/* ================== 对外暴露的接口 ================== */

void bsp_adc_init(bsp_adc_mode_enum mode)
{
    g_adc_mode = mode;
    
    if (mode == BSP_ADC_MODE_INTERNAL) {
        adc_internal_config();
    } else if (mode == BSP_ADC_MODE_DUAL_DMA) {
        adc_dual_dma_config();
    }
}

void bsp_adc_trigger(void)
{
    if (g_adc_mode == BSP_ADC_MODE_INTERNAL) {
        adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
    }
    /* 如果配置的是双 DMA 模式，硬件定时器会自动触发，无需软件干预 */
}

uint16_t bsp_adc_get_idata0(void) { return ADC_IDATA0(ADC0); }
uint16_t bsp_adc_get_idata1(void) { return ADC_IDATA1(ADC0); }
uint16_t bsp_adc_get_idata2(void) { return ADC_IDATA2(ADC0); }

uint32_t bsp_adc_get_dma_data0(void) { return adc_dma_value[0]; }
uint32_t bsp_adc_get_dma_data1(void) { return adc_dma_value[1]; }