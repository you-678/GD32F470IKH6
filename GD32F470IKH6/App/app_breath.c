#include "app_breath.h"

void app_breath_init(void)
{
    /* 初始化 PWM 硬件 */
    bsp_timer_pwm_init();
}

void app_breath_task(void)
{
    static uint16_t i = 0;
    static uint8_t  breathe_flag = 1;
    static uint32_t last_tick = 0;

    /* 利用 systick 记录时间，实现非阻塞的 5ms 延时 */
    /* 注意：如果你的 systick.c 里没有全局毫秒计数，可以直接用 delay_1ms(5)，
       但阻塞 5ms 对呼吸灯影响不大 */
    delay_1ms(5); 

    if (breathe_flag) {
        i++;
    } else {
        i--;
    }

    if (i > 500) {
        breathe_flag = 0;
    }
    if (i == 0) {
        breathe_flag = 1;
    }

    /* 更新占空比 */
    bsp_timer_pwm_set_duty(i);
}