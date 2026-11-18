#include "app_nb_led.h"
#include "bsp_led.h"
#include "systick.h"

/* 类型 A：有顺序动作（1→2→3→1），所以要 state 记住上次切到哪 */
typedef struct {
    uint8_t  enable;
    uint32_t tick;
    uint16_t interval;
    uint8_t  state;            /* 0/1/2 = 当前亮哪盏 */
} nb_led_handle_t;

static nb_led_handle_t led_flow;

void nb_led_start(uint16_t ms)
{
    led_flow.enable   = 1;
    led_flow.interval = ms;
    led_flow.tick     = uwTick;    /* 记下"现在几点" */
    led_flow.state    = 0;

    bsp_led_off(LED1);
    bsp_led_off(LED2);
    bsp_led_off(LED3);
    bsp_led_on(LED1);
}

void nb_led_process(void)
{
    if(!led_flow.enable) return;                      /* ① 没启用，直接走 */

    /* ② 瞄一眼钟：没到 500ms 就立刻返回，CPU 一毫秒都不占 */
    if(uwTick - led_flow.tick < led_flow.interval) return;

    led_flow.tick = uwTick;                           /* ③ 到点了，先记新时间（记 now，不记 +=interval） */

    /* ④ 到点才干一次活：切到下一盏灯 */
    switch(led_flow.state) {
    case 0: bsp_led_off(LED1); bsp_led_on(LED2);  led_flow.state = 1; break;
    case 1: bsp_led_off(LED2); bsp_led_on(LED3);  led_flow.state = 2; break;
    case 2: bsp_led_off(LED3); bsp_led_on(LED1);  led_flow.state = 0; break;
    default: led_flow.state = 0; break;
    }
}
