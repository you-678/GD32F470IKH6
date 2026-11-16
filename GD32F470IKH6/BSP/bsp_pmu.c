#include "bsp_pmu.h"

/*!
    \brief      初始化 PMU 时钟
*/
void bsp_pmu_init(void)
{
    rcu_periph_clock_enable(RCU_PMU);
}

/*!
    \brief      进入睡眠模式 (WFI)
*/
void bsp_enter_sleep_mode(void)
{
    /* 官方例程通过 WFI 指令进入 Sleep 模式 */
    pmu_to_sleepmode(WFI_CMD);
}