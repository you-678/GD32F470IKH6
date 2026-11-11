#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "gd32f4xx.h"

/*!
    \brief      按键编号枚举
      \arg        KEY_WAKEUP: 唤醒按键 (对应硬件引脚 PA0)
      \arg        KEY_TAMPER: 防拆按键 (对应硬件引脚 PC13)
      \arg        KEY_USER: 用户按键 (对应硬件引脚 PB14)
      \arg        KEYn: 按键总数 (用于静态数组映射)
*/
typedef enum { 
    KEY_WAKEUP = 0, 
    KEY_TAMPER,     
    KEY_USER,       
    KEYn            
} key_typedef_enum;

/*!
    \brief      按键工作模式枚举
      \arg        KEY_MODE_GPIO: 普通 GPIO 轮询模式
      \arg        KEY_MODE_EXTI: 外部中断 (EXTI) 触发模式
*/
typedef enum { 
    KEY_MODE_GPIO = 0, 
    KEY_MODE_EXTI      
} keymode_typedef_enum;

/*!
    \brief      按键中断回调函数指针类型
    \param[in]  keynum: 触发中断的按键编号
*/
typedef void (*key_isr_callback_t)(key_typedef_enum keynum);

/*!
    \brief      初始化按键（支持轮询或中断模式）
    \param[in]  keynum: 按键编号
      \arg        KEY_WAKEUP: 唤醒按键
      \arg        KEY_TAMPER: 防拆按键
      \arg        KEY_USER: 用户按键
    \param[in]  keymode: 按键工作模式
      \arg        KEY_MODE_GPIO: 轮询模式
      \arg        KEY_MODE_EXTI: 外部中断触发模式
    \param[out] 无
    \retval     无
*/
void bsp_key_init(key_typedef_enum keynum, keymode_typedef_enum keymode);

/*!
    \brief      获取按键当前的电平状态
    \param[in]  keynum: 按键编号
      \arg        KEY_WAKEUP: 唤醒按键
      \arg        KEY_TAMPER: 防拆按键
      \arg        KEY_USER: 用户按键
    \param[out] 无
    \retval     返回 0 表示按键按下(低电平)，返回 1 表示按键松开(高电平)
*/
uint8_t bsp_key_state_get(key_typedef_enum keynum);

/*!
    \brief      注册按键中断回调函数 (仅在外部中断模式下有效)
    \param[in]  keynum: 按键编号
      \arg        KEY_WAKEUP: 唤醒按键
      \arg        KEY_TAMPER: 防拆按键
      \arg        KEY_USER: 用户按键
    \param[in]  callback: 用户自定义的回调函数指针
    \param[out] 无
    \retval     无
*/
void bsp_key_register_callback(key_typedef_enum keynum, key_isr_callback_t callback);

/*!
    \brief      外部中断统一分发处理函数 (专供 gd32f4xx_it.c 中断服务函数调用)
    \param[in]  exti_line: 触发的EXTI中断线编号
      \arg        EXTI_0: 对应 PA0 中断线
      \arg        EXTI_13: 对应 PC13 中断线
      \arg        EXTI_14: 对应 PB14 中断线
    \param[out] 无
    \retval     无
*/
void bsp_key_irq_handler(uint32_t exti_line);

#endif