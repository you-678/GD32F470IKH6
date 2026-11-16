#include "bsp_timer.h"

/*!
    \brief      初始化 TIMER1 CH2 PWM 输出
*/
void bsp_timer_pwm_init(void)
{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    /* 1. 使能 GPIO 和 TIMER 时钟 */
    rcu_periph_clock_enable(BREATH_PWM_CLK);
    rcu_periph_clock_enable(RCU_TIMER1);
    
    /* 配置 APB1 定时器时钟倍频 */
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

    /* 2. 配置 GPIO 为复用推挽输出 */
    gpio_mode_set(BREATH_PWM_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BREATH_PWM_PIN);
    gpio_output_options_set(BREATH_PWM_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BREATH_PWM_PIN);
    gpio_af_set(BREATH_PWM_PORT, GPIO_AF_1, BREATH_PWM_PIN); /* TIMER1_CH2 对应 AF1 */

    /* 3. 配置 TIMER1 时基参数 */
    timer_deinit(TIMER1);

    timer_initpara.prescaler         = 119;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 500; /* 周期 500，对应占空比 0~500 */
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1, &timer_initpara);

    /* 4. 配置 TIMER1 CH2 PWM 模式 */
    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER1, TIMER_CH_2, &timer_ocintpara);

    /* 初始占空比为 0 */
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, 0);
    timer_channel_output_mode_config(TIMER1, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);

    /* 5. 使能自动重装载和定时器 */
    timer_auto_reload_shadow_enable(TIMER1);
    timer_enable(TIMER1);
}

/*!
    \brief      动态修改 PWM 占空比
    \param[in]  duty: 占空比脉冲值 (0 ~ 500)
*/
void bsp_timer_pwm_set_duty(uint16_t duty)
{
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, duty);
}