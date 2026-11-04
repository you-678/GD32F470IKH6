#include "bsp_key.h"
#include "systick.h"

#include "bsp_led.h"

void key_scan_task(void)
{
    /* 检测按键按下 */
    if(RESET == bsp_key_wakeup_read())
    {
        delay_1ms(100);    // 软件消抖
        if(RESET == bsp_key_wakeup_read())
        {
            bsp_led1_toggle();   // 翻转LED2
            /* 等待松手，防止连续触发（可选，原版例程没有松手等待，我保留原版行为） */
            while(RESET == bsp_key_wakeup_read());
        }
    }
		if(RESET == bsp_key_tamper_read())
    {
        delay_1ms(100);    // 软件消抖
        if(RESET == bsp_key_tamper_read())
        {
            bsp_led2_toggle();   // 翻转LED2
            /* 等待松手，防止连续触发（可选，原版例程没有松手等待，我保留原版行为） */
            while(RESET == bsp_key_tamper_read());
        }
    }
		if(RESET == bsp_key_user_read())
    {
        delay_1ms(100);    // 软件消抖
        if(RESET == bsp_key_user_read())
        {
            bsp_led3_toggle();   // 翻转LED2
            /* 等待松手，防止连续触发（可选，原版例程没有松手等待，我保留原版行为） */
            while(RESET == bsp_key_user_read());
        }
    }
}
