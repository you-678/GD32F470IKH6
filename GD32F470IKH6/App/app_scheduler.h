#ifndef __APP_SCHEDULER_H
#define __APP_SCHEDULER_H

#include <stdint.h>

/*
 * 非阻塞任务调度器（裸机）
 * 用法：在 app_scheduler.c 的 task_list[] 里注册各外设的 process 函数，
 *      主循环只调 task_scheduler() 一行。
 */
typedef void (*task_func_t)(void);

typedef struct {
    task_func_t proc;      /* 外设的 process 函数 */
    uint8_t     enable;    /* 1=参与调度，0=跳过 */
} task_item_t;

void task_scheduler(void);

#endif /* __APP_SCHEDULER_H */
