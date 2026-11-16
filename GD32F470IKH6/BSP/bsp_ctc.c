#include "bsp_ctc.h"

/*!
    \brief      初始化 CTC 硬件及相关时钟 (IRC48M, LXTAL)
*/
void bsp_ctc_init(void)
{
    /* 1. 使能 IRC48M 时钟 */
    RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
    while(0 == (RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB)) {
    }

    /* 2. 配置并开启 LXTAL (外部 32.768kHz 晶振) */
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();
    RCU_BDCTL |= RCU_BDCTL_LXTALEN;
    while(0 == (RCU_BDCTL & RCU_BDCTL_LXTALSTB)) {
    }

    /* 3. 使能 CTC 外设时钟 */
    rcu_periph_clock_enable(RCU_CTC);

    /* 4. 配置 CTC */
    /* 配置 CTC 参考信号源预分频 */
    ctc_refsource_prescaler_config(CTC_REFSOURCE_PSC_OFF);
    /* 选择参考信号源为 LXTAL */
    ctc_refsource_signal_select(CTC_REFSOURCE_LXTAL);
    /* 选择参考信号源极性为上升沿 */
    ctc_refsource_polarity_config(CTC_REFSOURCE_POLARITY_RISING);
    /* 配置为硬件自动校准模式 */
    ctc_hardware_trim_mode_config(CTC_HARDWARE_TRIM_MODE_ENABLE);

    /* 配置 CTC 计数器重装载值 */
    ctc_counter_reload_value_config(0x05B8);
    /* 配置时钟校准基础限制值 */
    ctc_clock_limit_value_config(0x0002);

    /* 使能 CTC 计数器 */
    ctc_counter_enable();
}

/*!
    \brief      检查时钟校准是否成功 (CKOK 标志位)
    \retval     1: 校准完成, 0: 尚未完成
*/
uint8_t bsp_ctc_is_trim_ok(void)
{
    if(RESET != ctc_flag_get(CTC_FLAG_CKOK)) {
        return 1;
    }
    return 0;
}