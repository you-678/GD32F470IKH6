#include "bsp_key_exti.h"

void bsp_key_exti_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* PC13  ‰»Î */
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_13);

    /* NVIC°¢SYSCFG°¢EXTI≈‰÷√ */
    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN13);
    exti_init(EXTI_13, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_flag_clear(EXTI_13);
}
