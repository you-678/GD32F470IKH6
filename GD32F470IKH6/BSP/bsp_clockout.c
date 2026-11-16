#include "bsp_clockout.h"

/*!
    \brief      初始化时钟输出相关的硬件
*/
void bsp_clockout_init(void)
{
    /* 使能相关 GPIO 和 PMU 时钟 */
    rcu_periph_clock_enable(CLKOUT0_CLK);
    rcu_periph_clock_enable(CLKOUT1_CLK);
    rcu_periph_clock_enable(RCU_PMU);

    /* 开启外部高速晶振 HXTAL */
    rcu_osci_on(RCU_HXTAL);
    if(ERROR == rcu_osci_stab_wait(RCU_HXTAL)) {
        printf("RCU_HXTAL rcu_osci_stab_wait timeout! \r\n");
        while(1);
    }

    /* 开启 PLLI2S */
    rcu_osci_on(RCU_PLLI2S_CK);
    if(ERROR == rcu_osci_stab_wait(RCU_PLLI2S_CK)) {
        printf("RCU_PLLI2S_CK rcu_osci_stab_wait timeout! \r\n");
        while(1);
    }

    /* 开启外部低速晶振 LXTAL (注意：如果板子没焊32.768k晶振，这里会卡死) */
    pmu_backup_write_enable();
    rcu_lxtal_drive_capability_config(RCU_LXTALDRI_HIGHER_DRIVE);
    rcu_osci_on(RCU_LXTAL);
    if(ERROR == rcu_osci_stab_wait(RCU_LXTAL)) {
        printf("RCU_LXTAL rcu_osci_stab_wait timeout! \r\n");
        while(1);
    }

    /* 配置 PA8 (CK_OUT0) 和 PC9 (CK_OUT1) 为复用推挽输出 */
    gpio_mode_set(CLKOUT0_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CLKOUT0_PIN);
    gpio_mode_set(CLKOUT1_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CLKOUT1_PIN);
    gpio_output_options_set(CLKOUT0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, CLKOUT0_PIN);
    gpio_output_options_set(CLKOUT1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, CLKOUT1_PIN);
    gpio_af_set(CLKOUT0_PORT, GPIO_AF_0, CLKOUT0_PIN);
    gpio_af_set(CLKOUT1_PORT, GPIO_AF_0, CLKOUT1_PIN);
}

/*!
    \brief      选择时钟输出源
    \param[in]  seq: 序号 (0, 1, 2)
*/
void bsp_clockout_select(uint8_t seq)
{
    switch(seq) {
    case CKOUT0_IRC16M_CKOUT1_SYSTEMCLOCK:
        rcu_ckout0_config(RCU_CKOUT0SRC_IRC16M, RCU_CKOUT0_DIV1);
        rcu_ckout1_config(RCU_CKOUT1SRC_SYSTEMCLOCK, RCU_CKOUT1_DIV5);
        printf("CK_OUT0: IRC16M, CK_OUT1: system clock/5 \r\n");
        break;
    case CKOUT0_LXTAL_CKOUT1_PLLI2SR:
        rcu_ckout0_config(RCU_CKOUT0SRC_LXTAL, RCU_CKOUT0_DIV1);
        rcu_ckout1_config(RCU_CKOUT1SRC_PLLI2SR, RCU_CKOUT1_DIV5);
        printf("CK_OUT0: LXTAL, CK_OUT1: PLLI2SR/5 \r\n");
        break;
    case CKOUT0_HXTAL_CKOUT1_HXTAL:
        rcu_ckout0_config(RCU_CKOUT0SRC_HXTAL, RCU_CKOUT0_DIV1);
        rcu_ckout1_config(RCU_CKOUT1SRC_HXTAL, RCU_CKOUT1_DIV1);
        printf("CK_OUT0: HXTAL, CK_OUT1: HXTAL \r\n");
        break;
    default:
        printf("clock output select parameter error! \r\n");
        break;
    }
}