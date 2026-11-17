#include "app_breath.h"

void app_breath_init(void)
{
    /* 初始化 PB10 PWM 硬件 */
    bsp_timer_pwm_init();
}

void app_breath_task(void)
{
    static uint16_t i = 0;
    static uint8_t  breathe_flag = 1;
    
    /* 官方原本的 5ms 延时 */
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