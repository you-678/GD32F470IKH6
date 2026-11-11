#ifndef __APP_KEY_H
#define __APP_KEY_H

#include "gd32f4xx.h"

/*!
    \brief      APP层按键初始化 (配置为轮询模式)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_key_init_polling(void);

/*!
    \brief      APP层按键初始化 (配置为中断模式并注册回调)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_key_init_exti(void);

/*!
    \brief      APP层按键轮询任务 (需在主循环中不断调用)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_key_polling_task(void);

#endif