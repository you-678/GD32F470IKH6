#include "bsp_led.h"

static rcu_periph_enum GPIO_CLK[LEDn] = {LED1_GPIO_CLK, LED2_GPIO_CLK, LED3_GPIO_CLK};
static uint32_t GPIO_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT, LED3_GPIO_PORT};
static uint32_t GPIO_PIN[LEDn] = {LED1_PIN, LED2_PIN, LED3_PIN};

void bsp_led_init(led_typedef_enum lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    /* configure led GPIO port */ 
    gpio_mode_set(GPIO_PORT[lednum], GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN[lednum]);
    gpio_output_options_set(GPIO_PORT[lednum], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[lednum]);

//    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	gpio_bit_reset(GPIO_PORT[lednum], GPIO_PIN[lednum]);
}


void bsp_led_on(led_typedef_enum lednum)
{
//    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	gpio_bit_set(GPIO_PORT[lednum], GPIO_PIN[lednum]);
}
void bsp_led_off(led_typedef_enum lednum)
{
//    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	gpio_bit_reset(GPIO_PORT[lednum], GPIO_PIN[lednum]);
}
void bsp_led_toggle(led_typedef_enum lednum)
{
//	GPIO_TG(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
	gpio_bit_toggle(GPIO_PORT[lednum], GPIO_PIN[lednum]);
}
