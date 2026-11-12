#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

/* 
 * 【切换开关】只需要改这里：BSP_I2C_MODE_HW 或 BSP_I2C_MODE_SW
 * 就能实现同一套代码硬件与软件 I2C 的随意切换
 */
#define TEST_I2C_MODE   BSP_I2C_MODE_HW

/*!
    \brief      系统主函数 - I2C EEPROM 测试
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    bsp_led_init(LED1); 
    bsp_led_init(LED2); 
    
    printf("\r\n========================================\r\n");
    printf("I2C EEPROM (AT24C02) Test Started!\r\n");
    printf("========================================\r\n");

    /* 1. 初始化 I2C (根据上面定义的宏切换模式) */
    bsp_i2c_init(TEST_I2C_MODE);

    /* 2. 执行自测 */
    if (app_i2c_eeprom_test(TEST_I2C_MODE) == 0) {
        /* 测试通过：快闪 3 次 LED1 */
        for(int i=0; i<3; i++) {
            bsp_led_toggle(LED1); delay_1ms(100);
        }
        printf("\r\n[Hardware] Test PASS. Run loop.\r\n");
    } else {
        /* 测试失败：快闪 3 次 LED2 */
        for(int i=0; i<3; i++) {
            bsp_led_toggle(LED2); delay_1ms(100);
        }
        printf("\r\n[Hardware] Test FAIL. Halted.\r\n");
    }

    while(1) {
        /* 测试已完成，停在这里 */
        delay_1ms(1000);
    }
}