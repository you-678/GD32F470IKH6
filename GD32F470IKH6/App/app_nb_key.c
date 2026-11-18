#include "app_nb_key.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "systick.h"
#include <stdio.h>

/* 类型 B：单纯周期性执行（扫一次按键），没有多步流程，不需要 state */
typedef struct {
    uint8_t  enable;
    uint32_t tick;
    uint16_t interval;
    uint8_t  last;        /* 上次电平：默认上拉，未按下=1，按下=0 */
    uint8_t  pressed;     /* 用标志代替 while 死等松手 */
} nb_key_handle_t;

static nb_key_handle_t key_scan;

void nb_key_start(uint16_t ms)
{
    key_scan.enable   = 1;
    key_scan.interval = ms;
    key_scan.tick     = uwTick;
    key_scan.last     = 1;        /* 上拉，空闲为高 */
    key_scan.pressed  = 0;
}

void nb_key_process(void)
{
    if(!key_scan.enable) return;
    if(uwTick - key_scan.tick < key_scan.interval) return;   /* 没到扫描周期，立刻走 */
    key_scan.tick = uwTick;

    uint8_t now = bsp_key_state_get(KEY_WAKEUP);   /* PA0：按下=0，松开=1 */

    if(now != key_scan.last) {          /* 电平变了才处理 */
        key_scan.last = now;
        if(now == 0 && !key_scan.pressed) {
            /* 下降沿 = 按下一次。这里消抖靠"5ms 扫一次+边沿"，不再 delay_1ms(20) */
            key_scan.pressed = 1;
            bsp_led_toggle(LED1);
            printf("[KEY] PA0 pressed!\r\n");
        } else if(now == 1) {
            key_scan.pressed = 0;       /* 松手靠这里记录，不再 while 死等 */
        }
    }
}
