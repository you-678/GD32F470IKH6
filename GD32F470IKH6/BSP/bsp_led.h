#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "gd32f4xx.h"

/*!
    \brief      指示灯编号枚举
      \arg        LED1: 指示灯1 (对应硬件引脚 PE2)
      \arg        LED2: 指示灯2 (对应硬件引脚 PE3)
      \arg        LED3: 指示灯3 (对应硬件引脚 PF10)
      \arg        LEDn: 指示灯总数 (用于静态数组映射)
*/
typedef enum { 
    LED1 = 0,   
    LED2,       
    LED3,       
    LEDn        
} led_typedef_enum;

/*!
    \brief      初始化指定的指示灯引脚
    \param[in]  lednum: 指示灯编号
                  只能选择以下参数之一：
      \arg        LED1: 指示灯1
      \arg        LED2: 指示灯2
      \arg        LED3: 指示灯3
    \param[out] 无
    \retval     无
*/
void bsp_led_init(led_typedef_enum lednum);

/*!
    \brief      点亮指定的指示灯
    \param[in]  lednum: 指示灯编号
      \arg        LED1: 指示灯1
      \arg        LED2: 指示灯2
      \arg        LED3: 指示灯3
    \param[out] 无
    \retval     无
*/
void bsp_led_on(led_typedef_enum lednum);

/*!
    \brief      熄灭指定的指示灯
    \param[in]  lednum: 指示灯编号
      \arg        LED1: 指示灯1
      \arg        LED2: 指示灯2
      \arg        LED3: 指示灯3
    \param[out] 无
    \retval     无
*/
void bsp_led_off(led_typedef_enum lednum);

/*!
    \brief      翻转指定的指示灯状态
    \param[in]  lednum: 指示灯编号
      \arg        LED1: 指示灯1
      \arg        LED2: 指示灯2
      \arg        LED3: 指示灯3
    \param[out] 无
    \retval     无
*/
void bsp_led_toggle(led_typedef_enum lednum);

#endif