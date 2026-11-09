#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "gd32f4xx.h"

typedef enum 
{
    LED1 = 0,
    LED2 = 1,
    LED3 = 2
} led_typedef_enum;

#define LEDn                             3U

#define LED1_PIN                         GPIO_PIN_2
#define LED1_GPIO_PORT                   GPIOE
#define LED1_GPIO_CLK                    RCU_GPIOE
  
#define LED2_PIN                         GPIO_PIN_3
#define LED2_GPIO_PORT                   GPIOE
#define LED2_GPIO_CLK                    RCU_GPIOE
  
#define LED3_PIN                         GPIO_PIN_10
#define LED3_GPIO_PORT                   GPIOF
#define LED3_GPIO_CLK                    RCU_GPIOF


void bsp_led_init(led_typedef_enum lednum);
void bsp_led_on(led_typedef_enum lednum);
void bsp_led_off(led_typedef_enum lednum);
void bsp_led_toggle(led_typedef_enum lednum);

#endif
