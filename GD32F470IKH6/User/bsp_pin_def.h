#ifndef __BSP_PIN_DEF_H
#define __BSP_PIN_DEF_H

#include "gd32f4xx.h"

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
#define BSP_DAC_GPIO_CLK    RCU_GPIOA
#define BSP_DAC_GPIO_PORT   GPIOA
#define BSP_DAC_PIN         GPIO_PIN_4

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

/* ============================================================================
 * 二、SPI Flash（GD25Qxx，硬件 SPI5）
 *   SCK  = PG13  (AF5)
 *   MISO = PG12  (AF5)
 *   MOSI = PG14  (AF5)
 *   IO2  = PG10  (AF5)   Quad 使用
 *   IO3  = PG11  (AF5)   Quad 使用
 *   CS   = PI8   (GPIO 推挽)
 * ========================================================================== */
#define BSP_SPI_FLASH_PERIPH      SPI5
#define BSP_SPI_FLASH_RCU         RCU_SPI5
#define BSP_SPI_FLASH_APB_HZ      BSP_APB2_CLOCK_HZ

#define BSP_SPI_FLASH_GPIO_RCU    RCU_GPIOG
#define BSP_SPI_FLASH_GPIO_PORT   GPIOG
#define BSP_SPI_FLASH_GPIO_AF     GPIO_AF_5

#define BSP_SPI_FLASH_SCK_PIN     GPIO_PIN_13
#define BSP_SPI_FLASH_MISO_PIN    GPIO_PIN_12
#define BSP_SPI_FLASH_MOSI_PIN    GPIO_PIN_14
#define BSP_SPI_FLASH_IO2_PIN     GPIO_PIN_10
#define BSP_SPI_FLASH_IO3_PIN     GPIO_PIN_11

#define BSP_SPI_FLASH_SINGLE_PINS (BSP_SPI_FLASH_SCK_PIN  | BSP_SPI_FLASH_MISO_PIN | BSP_SPI_FLASH_MOSI_PIN)
#define BSP_SPI_FLASH_QUAD_PINS   (BSP_SPI_FLASH_SINGLE_PINS | BSP_SPI_FLASH_IO2_PIN | BSP_SPI_FLASH_IO3_PIN)

#define BSP_SPI_FLASH_CS_RCU      RCU_GPIOI
#define BSP_SPI_FLASH_CS_PORT     GPIOI
#define BSP_SPI_FLASH_CS_PIN      GPIO_PIN_8

#define BSP_SPI_FLASH_PRESCALE    SPI_PSC_8                 /* 120MHz / 8 = 15MHz */
#define BSP_SPI_FLASH_CLK_HZ      (BSP_SPI_FLASH_APB_HZ / 8U)

/* ============================================================================
 * I2S 音频（I2S1 = SPI1）
 *   MCK = PA6  (AF6)
 *   BCK = PI1  (AF5)
 *   WS  = PI0  (AF5)
 *   SD  = PC1  (AF7)
 * ========================================================================== */
#define BSP_I2S_AUDIO_PERIPH       SPI1
#define BSP_I2S_AUDIO_RCU          RCU_SPI1
#define BSP_I2S_AUDIO_IRQn         SPI1_IRQn

#define BSP_I2S_MCK_RCU            RCU_GPIOA
#define BSP_I2S_MCK_PORT           GPIOA
#define BSP_I2S_MCK_PIN            GPIO_PIN_6
#define BSP_I2S_MCK_AF             GPIO_AF_6

#define BSP_I2S_CKWS_RCU           RCU_GPIOI
#define BSP_I2S_CKWS_PORT          GPIOI
#define BSP_I2S_CK_PIN             GPIO_PIN_1
#define BSP_I2S_WS_PIN             GPIO_PIN_0
#define BSP_I2S_CKWS_AF            GPIO_AF_5
#define BSP_I2S_CKWS_PINS          (BSP_I2S_CK_PIN | BSP_I2S_WS_PIN)

#define BSP_I2S_SD_RCU             RCU_GPIOC
#define BSP_I2S_SD_PORT            GPIOC
#define BSP_I2S_SD_PIN             GPIO_PIN_1
#define BSP_I2S_SD_AF              GPIO_AF_7

/* ============================================================================
 * 六、EXMC 外部存储总线（SDRAM + NAND 共用）
 * ========================================================================== */
#define BSP_EXMC_RCU              RCU_EXMC
#define BSP_SDRAM_BASE            ((uint32_t)0xC0000000U)
#define BSP_SDRAM_SIZE            (32U * 1024U * 1024U)

#define BSP_NAND_BASE             ((uint32_t)0x70000000U)
#define BSP_NAND_CMD_OFS          (1U << 16)
#define BSP_NAND_ADDR_OFS         (1U << 17)
#define BSP_NAND_DATA_OFS         (0U)

#define BSP_NAND_ZONE_COUNT       1U
#define BSP_NAND_ZONE_SIZE        1024U
#define BSP_NAND_BLOCK_SIZE       64U
#define BSP_NAND_PAGE_SIZE        2048U
#define BSP_NAND_SPARE_SIZE       128U
#define BSP_NAND_PAGE_TOTAL       (BSP_NAND_PAGE_SIZE + BSP_NAND_SPARE_SIZE)
#define BSP_NAND_BLOCK_COUNT      (BSP_NAND_ZONE_COUNT * BSP_NAND_ZONE_SIZE)
#define BSP_NAND_MAX_ADDR         (BSP_NAND_BLOCK_COUNT * BSP_NAND_BLOCK_SIZE * BSP_NAND_PAGE_SIZE)
#endif