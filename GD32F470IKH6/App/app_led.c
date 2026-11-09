#include "app_led.h"

void app_led_all_on(void)
{
	bsp_led_on(LED1);bsp_led_on(LED2);bsp_led_on(LED3);
}
void app_led_all_off(void)
{
	bsp_led_off(LED1);bsp_led_off(LED2);bsp_led_off(LED3);
}
void app_led_all_toggle(void)
{
	bsp_led_toggle(LED1);bsp_led_toggle(LED2);bsp_led_toggle(LED3);
}
void app_led_flash(uint8_t times)
{
    uint8_t i;
    for(i = 0; i < times; i++)
    {
        delay_1ms(500);
        bsp_led_on(LED1);bsp_led_on(LED2);bsp_led_on(LED3);
        delay_1ms(500);
        bsp_led_off(LED1);bsp_led_off(LED2);bsp_led_off(LED3);
    }
}

void app_led_flow_task(void)
{
    /* LED1亮，LED3灭 */
    bsp_led_on(LED1);
    bsp_led_off(LED3);
    delay_1ms(1000);

    /* LED2亮，LED1灭 */
    bsp_led_on(LED2);
    bsp_led_off(LED1);
    delay_1ms(1000);

    /* LED3亮，LED2灭 */
    bsp_led_on(LED3);
    bsp_led_off(LED2);
    delay_1ms(1000);
}

