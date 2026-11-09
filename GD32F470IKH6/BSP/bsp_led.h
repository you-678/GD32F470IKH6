#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "gd32f4xx.h"

// 硬件定义，只在这里改引脚！
#define LED1_PORT    GPIOE
#define LED1_PIN     GPIO_PIN_2
#define LED2_PORT    GPIOE
#define LED2_PIN     GPIO_PIN_3
#define LED3_PORT    GPIOF
#define LED3_PIN     GPIO_PIN_10

void bsp_led_init(void);
void bsp_led1_on(void);
void bsp_led1_off(void);
void bsp_led2_on(void);
void bsp_led2_off(void);
void bsp_led3_on(void);
void bsp_led3_off(void);

void bsp_led1_toggle(void);
void bsp_led2_toggle(void);
void bsp_led3_toggle(void);

void bsp_led_all_off(void);
#endif
