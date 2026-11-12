#ifndef __BSP_PIN_DEF_H
#define __BSP_PIN_DEF_H

#include "gd32f4xx.h"

/*!
    \brief      硬件引脚与时钟映射宏定义
    \note       换板子时，只需修改此文件，无需改动驱动层和业务层代码
*/

/* ====== LED 硬件定义 ====== */
#define LED1_GPIO_CLK      RCU_GPIOE
#define LED1_GPIO_PORT     GPIOE
#define LED1_PIN           GPIO_PIN_2

#define LED2_GPIO_CLK      RCU_GPIOE
#define LED2_GPIO_PORT     GPIOE
#define LED2_PIN           GPIO_PIN_3

#define LED3_GPIO_CLK      RCU_GPIOF
#define LED3_GPIO_PORT     GPIOF
#define LED3_PIN           GPIO_PIN_10

/* ====== KEY 硬件定义 ====== */
#define KEY_WAKEUP_CLK     RCU_GPIOA
#define KEY_WAKEUP_PORT    GPIOA
#define KEY_WAKEUP_PIN     GPIO_PIN_0

#define KEY_TAMPER_CLK     RCU_GPIOC
#define KEY_TAMPER_PORT    GPIOC
#define KEY_TAMPER_PIN     GPIO_PIN_13

#define KEY_USER_CLK       RCU_GPIOB
#define KEY_USER_PORT      GPIOB
#define KEY_USER_PIN       GPIO_PIN_14

/* ====== USART 硬件定义 (串口0) ====== */
#define BSP_USART0_CLK        RCU_USART0
#define BSP_USART0_GPIO_CLK   RCU_GPIOA
#define BSP_USART0_GPIO_PORT  GPIOA
#define BSP_USART0_TX_PIN     GPIO_PIN_9
#define BSP_USART0_RX_PIN     GPIO_PIN_10
#define BSP_USART0_AF         GPIO_AF_7
#define BSP_USART0_PERIPH     USART0
#define BSP_USART0_IRQn       USART0_IRQn

/* ====== ADC 硬件定义 ====== */
#define BSP_ADC_DUAL_GPIO_CLK   RCU_GPIOC
#define BSP_ADC_DUAL_GPIO_PORT  GPIOC
#define BSP_ADC_CH13_PIN        GPIO_PIN_3
#define BSP_ADC_CH15_PIN        GPIO_PIN_5

/* ====== DAC 硬件定义 ====== */
/* ====== DAC 硬件定义 ====== */
#define BSP_DAC_GPIO_CLK    RCU_GPIOA
#define BSP_DAC_GPIO_PORT   GPIOA
#define BSP_DAC_PIN         GPIO_PIN_4

/* 当前库中，DAC0 直接代表输出通道0 */
#define BSP_DAC_PERIPH      DAC0 

/* =========================================================
 * 6. I2C / EEPROM 硬件映射
 * ========================================================= */
#define BSP_I2C_PERIPH          I2C0
#define BSP_I2C_RCU             RCU_I2C0
#define BSP_I2C_GPIO_RCU        RCU_GPIOB
#define BSP_I2C_GPIO_PORT       GPIOB

#define BSP_I2C_SCL_PIN         GPIO_PIN_6
#define BSP_I2C_SDA_PIN         GPIO_PIN_7
#define BSP_I2C_AF              GPIO_AF_4

#define BSP_EEPROM_ADDR         0xA0    /* AT24C02 默认地址 */
#define BSP_EEPROM_PAGE_SIZE    8       /* AT24C02 页大小为8字节 */

#endif