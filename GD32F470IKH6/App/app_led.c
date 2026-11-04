#include "app_led.h"
#include "bsp_led.h"
#include "systick.h"

void led_flow_task(void)
{
    /* LED1亮，LED3灭 */
    bsp_led1_on();
    bsp_led3_off();
    delay_1ms(1000);

    /* LED2亮，LED1灭 */
    bsp_led2_on();
    bsp_led1_off();
    delay_1ms(1000);

    /* LED3亮，LED2灭 */
    bsp_led3_on();
    bsp_led2_off();
    delay_1ms(1000);
}

/* 上电自检闪烁函数，对应原版led_flash */
void app_led_flash(uint8_t times)
{
    uint8_t i;
    for(i = 0; i < times; i++)
    {
        delay_1ms(500);
        bsp_led2_on();
        delay_1ms(500);
        bsp_led2_off();
    }
}

/* 中断触发调用：翻转LED */
void app_led_toggle(void)
{
    bsp_led2_toggle();
}