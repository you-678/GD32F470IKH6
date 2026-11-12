#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    printf("\r\n\r\n");
    printf("========================================\r\n");
    printf("  Stage 1: Test Hardware I2C (HW)       \r\n");
    printf("========================================\r\n");
    
    /* 1. 测试硬件 I2C */
    app_i2c_eeprom_init(BSP_I2C_MODE_HW);
    delay_1ms(1000); /* 稍作延时，避免打印太快 */


    printf("\r\n\r\n");
    printf("========================================\r\n");
    printf("  Stage 2: Test Software I2C (SW)       \r\n");
    printf("========================================\r\n");
    
    /* 2. 测试软件 I2C */
    app_i2c_eeprom_init(BSP_I2C_MODE_SW);
    delay_1ms(1000);


    printf("\r\n\r\n");
    printf("========================================\r\n");
    printf("  Both HW & SW Tested. Entering Loop    \r\n");
    printf("========================================\r\n");

    /* 3. 进入主循环 (跑马灯或全亮，取决于最后一次测试结果) */
    while (1) {
        app_i2c_eeprom_task();
    }
}