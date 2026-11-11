#include "bsp_key.h"

static const rcu_periph_enum GPIO_CLK[KEYn]  = {KEY_WAKEUP_CLK, KEY_TAMPER_CLK, KEY_USER_CLK};
static const uint32_t GPIO_PORT[KEYn]        = {KEY_WAKEUP_PORT, KEY_TAMPER_PORT, KEY_USER_PORT};
static const uint32_t GPIO_PIN[KEYn]         = {KEY_WAKEUP_PIN, KEY_TAMPER_PIN, KEY_USER_PIN};
static const uint8_t  EXTI_PORT_SOURCE[KEYn] = {EXTI_SOURCE_GPIOA, EXTI_SOURCE_GPIOC, EXTI_SOURCE_GPIOB};
static const uint8_t  EXTI_PIN_SOURCE[KEYn]  = {EXTI_SOURCE_PIN0, EXTI_SOURCE_PIN13, EXTI_SOURCE_PIN14};
static const uint32_t EXTI_LINE[KEYn]        = {EXTI_0, EXTI_13, EXTI_14};

static key_isr_callback_t key_callback[KEYn] = {0};

/*!
    \brief      初始化按键（支持轮询或中断模式）
    \param[in]  keynum: 按键编号
    \param[in]  keymode: 模式 (轮询或中断)
    \param[out] 无
    \retval     无
*/
void bsp_key_init(key_typedef_enum keynum, keymode_typedef_enum keymode) {
    if (keynum >= KEYn) return;
    rcu_periph_clock_enable(GPIO_CLK[keynum]);
    gpio_mode_set(GPIO_PORT[keynum], GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN[keynum]);

    if (keymode == KEY_MODE_EXTI) {
        rcu_periph_clock_enable(RCU_SYSCFG);
        syscfg_exti_line_config(EXTI_PORT_SOURCE[keynum], EXTI_PIN_SOURCE[keynum]);
        exti_init(EXTI_LINE[keynum], EXTI_INTERRUPT, EXTI_TRIG_RISING);
        exti_interrupt_flag_clear(EXTI_LINE[keynum]);
        
        if (keynum == KEY_WAKEUP) {
            nvic_irq_enable(EXTI0_IRQn, 2U, 0U);
        } else {
            nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
        }
    }
}

/*!
    \brief      获取按键当前的电平状态
    \param[in]  keynum: 按键编号
    \param[out] 无
    \retval     0表示按下(低电平)，1表示松开(高电平)
*/
uint8_t bsp_key_state_get(key_typedef_enum keynum) { 
    return (keynum < KEYn) ? gpio_input_bit_get(GPIO_PORT[keynum], GPIO_PIN[keynum]) : 1; 
}

/*!
    \brief      注册按键中断回调函数
    \param[in]  keynum: 按键编号
    \param[in]  callback: 回调函数指针
    \param[out] 无
    \retval     无
*/
void bsp_key_register_callback(key_typedef_enum keynum, key_isr_callback_t callback) { 
    if (keynum < KEYn) key_callback[keynum] = callback; 
}

/*!
    \brief      外部中断统一分发处理函数
    \param[in]  exti_line: 触发的EXTI中断线编号
    \param[out] 无
    \retval     无
*/
void bsp_key_irq_handler(uint32_t exti_line) {
    for (uint8_t i = 0; i < KEYn; i++) {
        if (exti_line == EXTI_LINE[i] && exti_interrupt_flag_get(EXTI_LINE[i]) != RESET) {
            exti_interrupt_flag_clear(EXTI_LINE[i]);
            if (key_callback[i]) key_callback[i]((key_typedef_enum)i);
        }
    }
}