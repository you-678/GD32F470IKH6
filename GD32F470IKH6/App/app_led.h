#ifndef __APP_LED_H
#define __APP_LED_H

#include "gd32f4xx.h"

/*!
    \brief      点亮所有指示灯
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_all_on(void);

/*!
    \brief      熄灭所有指示灯
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_all_off(void);

/*!
    \brief      翻转所有指示灯状态
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_all_toggle(void);

/*!
    \brief      所有指示灯闪烁指定次数 (阻塞式)
    \param[in]  times: 闪烁次数 (例如: 2表示亮灭闪2次)
    \param[out] 无
    \retval     无
*/
void app_led_flash(uint8_t times);

/*!
    \brief      指示灯流水灯任务 (阻塞式，循环跑一圈)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_flow_task(void);

#endif