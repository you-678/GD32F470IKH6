#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "gd32f4xx.h"

#define KEY_TAMPER_PORT   GPIOC
#define KEY_TAMPER_PIN    GPIO_PIN_13

void bsp_key_init(void);
uint8_t bsp_key_tamper_read(void);  // 返回按键原始电平

#endif
