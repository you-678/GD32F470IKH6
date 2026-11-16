#include "bsp_rtc.h"

rtc_parameter_struct rtc_initpara;
__IO uint32_t prescaler_a = 0, prescaler_s = 0;

/*!
    \brief      初始化 RTC 硬件（配置时钟源和预分频）
*/
void bsp_rtc_init(void)
{
    /* 开启 PMU 时钟并允许访问备份域 */
    rcu_periph_clock_enable(RCU_PMU);
    pmu_backup_write_enable();

    /* 使用外部 LXTAL 作为 RTC 时钟源 */
    rcu_osci_on(RCU_LXTAL);
    rcu_osci_stab_wait(RCU_LXTAL);
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    prescaler_s = 0xFF;
    prescaler_a = 0x7F;

    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();
}

/*!
    \brief      检查 RTC 是否已经配置过（通过读取备份寄存器 BKP0）
    \retval     1: 已配置过, 0: 第一次上电
*/
uint8_t bsp_rtc_is_configured(void)
{
    if (RTC_BKP0 == RTC_BKP_MAGIC) {
        return 1;
    }
    return 0;
}

/*!
    \brief      标记 RTC 已经配置完成
*/
void bsp_rtc_mark_configured(void)
{
    RTC_BKP0 = RTC_BKP_MAGIC;
}

/*!
    \brief      设置当前时间
    \param[in]  hour: 小时 (0~23 的 BCD 码)
    \param[in]  minute: 分钟 (0~59 的 BCD 码)
    \param[in]  second: 秒 (0~59 的 BCD 码)
*/
void bsp_rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second)
{
    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn = prescaler_s;
    
    /* 默认日期：2026年4月30日 星期六 */
    rtc_initpara.year = 0x26; 
    rtc_initpara.day_of_week = RTC_SATURDAY;
    rtc_initpara.month = RTC_APR;
    rtc_initpara.date = 0x30;
    
    rtc_initpara.display_format = RTC_24HOUR;
    rtc_initpara.am_pm = RTC_AM;
    
    rtc_initpara.hour = hour;
    rtc_initpara.minute = minute;
    rtc_initpara.second = second;

    if(ERROR == rtc_init(&rtc_initpara)) {
        printf("\r\n[RTC] Config Failed!\r\n");
    } else {
        printf("\r\n[RTC] Config Success!\r\n");
        bsp_rtc_mark_configured();
    }
}

/*!
    \brief      打印当前时间
*/
void bsp_rtc_show_time(void)
{
    rtc_current_time_get(&rtc_initpara);
    printf("[RTC] Current time: %0.2x:%0.2x:%0.2x\r\n", 
           rtc_initpara.hour, rtc_initpara.minute, rtc_initpara.second);
}