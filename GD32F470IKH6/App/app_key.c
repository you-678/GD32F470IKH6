#include "bsp_key.h"

void app_key_scan_task(void)
{
    /* 检测按键按下 */
    if(RESET == bsp_key_state_get(KEY_WAKEUP))
    {
        delay_1ms(100);    // 软件消抖
        if(RESET == bsp_key_state_get(KEY_WAKEUP))
        {
            bsp_led_toggle(LED1);   // 翻转LED2
            /* 等待松手，防止连续触发（可选，原版例程没有松手等待，我保留原版行为） */
            while(RESET == bsp_key_state_get(KEY_WAKEUP));
        }
    }
		if(RESET == bsp_key_state_get(KEY_TAMPER))
    {
        delay_1ms(100);    // 软件消抖
        if(RESET == bsp_key_state_get(KEY_TAMPER))
        {
            bsp_led_toggle(LED2);   // 翻转LED2
            /* 等待松手，防止连续触发（可选，原版例程没有松手等待，我保留原版行为） */
            while(RESET == bsp_key_state_get(KEY_TAMPER));
        }
    }
		if(RESET == bsp_key_state_get(KEY_USER))
    {
        delay_1ms(100);    // 软件消抖
        if(RESET == bsp_key_state_get(KEY_USER))
        {
            bsp_led_toggle(LED3);   // 翻转LED2
            /* 等待松手，防止连续触发（可选，原版例程没有松手等待，我保留原版行为） */
            while(RESET == bsp_key_state_get(KEY_USER));
        }
    }
}
