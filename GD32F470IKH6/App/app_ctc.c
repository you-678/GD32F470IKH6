#include "app_ctc.h"

void app_ctc_init(void)
{
    printf("\r\n[CTC] Start calibrating...\r\n");
    /* 初始化硬件 */
    bsp_ctc_init();
}

void app_ctc_task(void)
{
    static uint8_t is_printed = 0;
    
    /* 如果校准成功，点亮 LED1 */
    if (bsp_ctc_is_trim_ok()) {
        bsp_led_on(LED1);
        
        /* 只在刚校准成功时打印一次，防止刷屏 */
        if (!is_printed) {
            printf("[CTC] Calibration OK! IRC48M is trimmed.\r\n");
            is_printed = 1;
        }
    } else {
        bsp_led_off(LED1);
    }

    /* 为了让主循环有事可做，加点肉眼可见的反馈 */
    static uint32_t tick = 0;
    tick++;
    if (tick > 1000000) { 
        tick = 0;
        bsp_led_toggle(LED2); /* 让 LED2 闪烁，证明主循环活着 */
    }
}