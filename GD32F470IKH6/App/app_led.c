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
