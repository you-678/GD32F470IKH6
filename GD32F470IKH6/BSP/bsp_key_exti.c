#include "bsp_key_exti.h"

void bsp_key_exti_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
		rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* PC13 输入 */
    gpio_mode_set(KEY_WAKEUP_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, KEY_WAKEUP_PIN);
		gpio_mode_set(KEY_TAMPER_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, KEY_TAMPER_PIN);
    gpio_mode_set(KEY_USER_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, KEY_USER_PIN);

		/* NVIC：EXTI10~15 共用一个中断，只需要使能一次；EXTI0 单独使能 */
    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
    nvic_irq_enable(EXTI0_IRQn,     2U, 0U);
	
    /* ---------- WAKEUP_KEY：PA0 / EXTI_0 ---------- */
    syscfg_exti_line_config(WAKEUP_KEY_EXTI_PORT_SOURCE, WAKEUP_KEY_EXTI_PIN_SOURCE);
    exti_init(WAKEUP_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(WAKEUP_KEY_EXTI_LINE);

    /* ---------- TAMPER_KEY：PC13 / EXTI_13 ---------- */
    syscfg_exti_line_config(TAMPER_KEY_EXTI_PORT_SOURCE, TAMPER_KEY_EXTI_PIN_SOURCE);
    exti_init(TAMPER_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(TAMPER_KEY_EXTI_LINE);

    /* ---------- USER_KEY：PB14 / EXTI_14 ---------- */
    syscfg_exti_line_config(USER_KEY_EXTI_PORT_SOURCE, USER_KEY_EXTI_PIN_SOURCE);
    exti_init(USER_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(USER_KEY_EXTI_LINE);
}
