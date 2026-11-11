#ifndef __APP_USART_H
#define __APP_USART_H

#include "gd32f4xx.h"

/*!
    \brief      串口综合应用任务 (处理接收、命令解析、DMA回显，需在主循环中不断调用)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_usart_task(void);

#endif