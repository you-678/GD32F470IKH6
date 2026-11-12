#include "bsp_dac.h"

/*!
    \brief      初始化 DAC 硬件
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void bsp_dac_init(void) {
    /* 1. 开启 GPIO 和 DAC 时钟 */
    rcu_periph_clock_enable(BSP_DAC_GPIO_CLK);
    rcu_periph_clock_enable(RCU_DAC);

    /* 2. 配置引脚为模拟模式 */
    gpio_mode_set(BSP_DAC_GPIO_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_DAC_PIN);

    /* 3. 初始化 DAC (你的库无参) */
    dac_deinit(); 
    
    /* 4. 各种配置 (去掉通道参数) */
    dac_trigger_disable(BSP_DAC_PERIPH);
    dac_wave_mode_config(BSP_DAC_PERIPH, DAC_WAVE_DISABLE);
    dac_output_buffer_enable(BSP_DAC_PERIPH);
    dac_enable(BSP_DAC_PERIPH);
    
    /* 5. 初始输出 0V (你的库只要 3 个参数) */
    dac_data_set(BSP_DAC_PERIPH, DAC_ALIGN_12B_R, 0);
}

/*!
    \brief      设置 DAC 输出电压
    \param[in]  value: 12位数字量 (0~4095)
    \param[out] 无
    \retval     无
*/
void bsp_dac_set_value(uint16_t value) {
    if (value > 4095) {
        value = 4095;
    }
    /* 只需要 3 个参数 */
    dac_data_set(BSP_DAC_PERIPH, DAC_ALIGN_12B_R, value);
}