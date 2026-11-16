#include "app_clockout.h"

void app_clockout_init(void)
{
    printf("\r\n /=========== Clock output Demo ===========/ \r\n");
    printf("press Tamper key to select clock output source \r\n");
    
    /* 初始化时钟输出硬件 */
    bsp_clockout_init();
    
    /* 先默认输出一个模式，方便示波器直接测到波形 */
    bsp_clockout_select(0);
}

void app_clockout_task(void)
{
    static uint8_t func_seq = 0;
    static uint8_t last_key_state = 1;

    /* 读取当前的按键状态 (假设上拉输入，未按下为1，按下为0) */
    uint8_t current_key_state = bsp_key_state_get(KEY_TAMPER);

    /* 边缘检测：上一次是高电平，这一次是低电平，说明按下了 */
    if (current_key_state == 0 && last_key_state == 1) {
        delay_1ms(20); /* 简单消抖 */
        
        if (bsp_key_state_get(KEY_TAMPER) == 0) {
            /* 1. 切换时钟源 */
            bsp_clockout_select(func_seq);
            
            /* 2. 更新 LED 指示 */
            bsp_led_off(LED1);
            bsp_led_off(LED2);
            bsp_led_off(LED3);
            bsp_led_on((led_typedef_enum)func_seq);
            
            /* 3. 序号递增 */
            func_seq++;
            func_seq %= CKOUT_COUNT;
        }
    }
    
    /* 记录本次状态，供下次循环使用 */
    last_key_state = current_key_state;
}