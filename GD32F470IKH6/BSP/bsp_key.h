#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "gd32f4xx.h"

/* 保持你原有的枚举不变 */
typedef enum {
    KEY_WAKEUP = 0,
    KEY_TAMPER,
    KEY_USER,
    KEYn
} key_typedef_enum;

typedef enum {
    KEY_MODE_GPIO = 0,
    KEY_MODE_EXTI
} keymode_typedef_enum;

typedef void (*key_isr_callback_t)(key_typedef_enum keynum);

/* ======= 新增：按键电平触发模式 ======= */
typedef enum {
    KEY_TRIG_FALLING_PULLUP = 0, /* 默认：上拉输入，下降沿（按下接 GND） */
    KEY_TRIG_RISING_FLOATING = 1 /* 浮空输入，上升沿（按下接 3.3V） */
} key_trigger_typedef_enum;

/* 新增：动态设置触发模式 */
void bsp_key_set_trigger(key_trigger_typedef_enum trigger);

/* 保持原有函数声明 */
void bsp_key_init(key_typedef_enum keynum, keymode_typedef_enum keymode);
uint8_t bsp_key_state_get(key_typedef_enum keynum);
void bsp_key_register_callback(key_typedef_enum keynum, key_isr_callback_t callback);
void bsp_key_irq_handler(uint32_t exti_line);

#endif