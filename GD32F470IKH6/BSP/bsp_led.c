#include "bsp_led.h"

void bsp_led_init(void)
{
    /* 开启时钟 */
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_GPIOF);

    /* LED1 PE2 */
    gpio_mode_set(LED1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED1_PIN);
    gpio_output_options_set(LED1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED1_PIN);
    gpio_bit_reset(LED1_PORT, LED1_PIN);

    /* LED2 PE3 */
    gpio_mode_set(LED2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED2_PIN);
    gpio_output_options_set(LED2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED2_PIN);
    gpio_bit_reset(LED2_PORT, LED2_PIN);

    /* LED3 PF10 跳线帽JP15要跳到LED3 */
    gpio_mode_set(LED3_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED3_PIN);
    gpio_output_options_set(LED3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LED3_PIN);
    gpio_bit_reset(LED3_PORT, LED3_PIN);
}

void bsp_led1_on(void)
{
    gpio_bit_set(LED1_PORT, LED1_PIN);
}
void bsp_led1_off(void)
{
    gpio_bit_reset(LED1_PORT, LED1_PIN);
}

void bsp_led2_on(void)
{
    gpio_bit_set(LED2_PORT, LED2_PIN);
}
void bsp_led2_off(void)
{
    gpio_bit_reset(LED2_PORT, LED2_PIN);
}

void bsp_led3_on(void)
{
    gpio_bit_set(LED3_PORT, LED3_PIN);
}
void bsp_led3_off(void)
{
    gpio_bit_reset(LED3_PORT, LED3_PIN);
}

void bsp_led1_toggle(void)
{
    gpio_bit_write(LED1_PORT, LED1_PIN, !gpio_input_bit_get(LED1_PORT, LED1_PIN));
}

void bsp_led2_toggle(void)
{
    gpio_bit_write(LED2_PORT, LED2_PIN, !gpio_input_bit_get(LED2_PORT, LED2_PIN));
}

void bsp_led3_toggle(void)
{
    gpio_bit_write(LED3_PORT, LED3_PIN, !gpio_input_bit_get(LED3_PORT, LED3_PIN));
}
