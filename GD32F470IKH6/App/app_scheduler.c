#include "app_scheduler.h"

/* 各外设 process 声明：谁要进调度，就在这里 extern 一下 */
extern void nb_key_process(void);   /* 按键扫描（响应优先，放最前） */
extern void nb_led_process(void);   /* LED 流水灯 */

/*
 * 任务数组：新增外设只需加一行 {函数, 1}；不需要改主循环。
 * 顺序 = 同一轮里谁先被服务（不是抢占优先级）。按键放前面，响应更快。
 * 事件型的 app_usart_task() 不进这里——它收到才处理，没有定时需求。
 */
static task_item_t task_list[] = {
    { nb_key_process, 1 },
    { nb_led_process, 1 },
};

#define TASK_CNT (sizeof(task_list) / sizeof(task_list[0]))

void task_scheduler(void)
{
    for(uint8_t i = 0; i < TASK_CNT; i++) {
        if(task_list[i].enable && task_list[i].proc) {
            task_list[i].proc();     /* 每个 process 自己看 uwTick 决定动不动，快进快出 */
        }
    }
}
