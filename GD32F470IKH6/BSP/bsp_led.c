#include "bsp_led.h"

static const rcu_periph_enum GPIO_CLK[LEDn]  = { LED1_GPIO_CLK, LED2_GPIO_CLK, LED3_GPIO_CLK };
static const uint32_t        GPIO_PORT[LEDn] = { LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT };
static const uint32_t        GPIO_PIN[LEDn]  = { LED1_PIN, LED2_PIN, LED3_PIN };

/*!
    \brief      初始化指定的指示灯引脚
    \param[in]  lednum: 指示灯编号 (LED1~LED3)
    \param[out] 无
    \retval     无
*/
void bsp_led_init(led_typedef_enum lednum) {
    if (lednum >= LEDn) return;
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    gpio_mode_set(GPIO_PORT[lednum], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN[lednum]);
    gpio_output_options_set(GPIO_PORT[lednum], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN[lednum]);
    gpio_bit_reset(GPIO_PORT[lednum], GPIO_PIN[lednum]); 
}

/*!
    \brief      点亮指定的指示灯
    \param[in]  lednum: 指示灯编号 (LED1~LED3)
    \param[out] 无
    \retval     无
*/
void bsp_led_on(led_typedef_enum lednum) { 
    if (lednum < LEDn) gpio_bit_set(GPIO_PORT[lednum], GPIO_PIN[lednum]); 
}

/*!
    \brief      熄灭指定的指示灯
    \param[in]  lednum: 指示灯编号 (LED1~LED3)
    \param[out] 无
    \retval     无
*/
void bsp_led_off(led_typedef_enum lednum) { 
    if (lednum < LEDn) gpio_bit_reset(GPIO_PORT[lednum], GPIO_PIN[lednum]); 
}

/*!
    \brief      翻转指定的指示灯状态
    \param[in]  lednum: 指示灯编号 (LED1~LED3)
    \param[out] 无
    \retval     无
*/
void bsp_led_toggle(led_typedef_enum lednum) { 
    if (lednum < LEDn) gpio_bit_toggle(GPIO_PORT[lednum], GPIO_PIN[lednum]); 
}