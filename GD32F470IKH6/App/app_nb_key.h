#ifndef __APP_NB_KEY_H
#define __APP_NB_KEY_H

#include <stdint.h>

void nb_key_start(uint16_t ms);   /* 启动按键扫描，ms=扫描周期，建议 5 */
void nb_key_process(void);        /* 放进调度器轮询，非阻塞 */

#endif /* __APP_NB_KEY_H */
