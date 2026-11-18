#ifndef __APP_NB_LED_H
#define __APP_NB_LED_H

#include <stdint.h>

void nb_led_start(uint16_t ms);   /* 启动流水灯，ms=每盏灯亮多久 */
void nb_led_process(void);        /* 放进调度器轮询，非阻塞 */

#endif /* __APP_NB_LED_H */
