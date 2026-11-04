#ifndef __BSP_KEY_EXTI_H
#define __BSP_KEY_EXTI_H

#include "gd32f4xx.h"

#define TAMPER_KEY_EXTI_LINE             EXTI_13
#define TAMPER_KEY_EXTI_PORT_SOURCE      EXTI_SOURCE_GPIOC
#define TAMPER_KEY_EXTI_PIN_SOURCE       EXTI_SOURCE_PIN13
#define TAMPER_KEY_EXTI_IRQn             EXTI10_15_IRQn

#define WAKEUP_KEY_EXTI_LINE             EXTI_0
#define WAKEUP_KEY_EXTI_PORT_SOURCE      EXTI_SOURCE_GPIOA
#define WAKEUP_KEY_EXTI_PIN_SOURCE       EXTI_SOURCE_PIN0
#define WAKEUP_KEY_EXTI_IRQn             EXTI0_IRQn  

#define USER_KEY_EXTI_LINE               EXTI_14
#define USER_KEY_EXTI_PORT_SOURCE        EXTI_SOURCE_GPIOB
#define USER_KEY_EXTI_PIN_SOURCE         EXTI_SOURCE_PIN14
#define USER_KEY_EXTI_IRQn               EXTI10_15_IRQn

void bsp_key_exti_init(void);

#endif
