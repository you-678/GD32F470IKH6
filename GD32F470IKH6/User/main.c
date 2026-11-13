#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"

int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);

    printf("\r\n========================================\r\n");
    printf("GD32F470 Full Test Boot\r\n");
    printf("========================================\r\n");

    /* =========================================================
     * 阶段一：按键轮询测试 (5 秒)
     * ========================================================= */
    printf("\r\n[STAGE 1] POLLING TEST (5 seconds)...\r\n");
    printf("Press PA0/PC13/PB14 to toggle LEDs.\r\n");

    app_key_init_polling();
    for (uint32_t i = 0; i < 5000; i++) {
        app_key_polling_task();
        delay_1ms(1);
    }

    /* =========================================================
     * 阶段二：EXMC 三项存储测试 (SDRAM -> SLEEP -> NAND)
     * ========================================================= */
    printf("\r\n[STAGE 2] EXMC STORAGE TEST...\r\n");
    
    for (int m = 0; m < STORAGE_MODE_MAX; m++) {
        drv_storage_run((storage_mode_t)m);
        delay_1ms(1000);
    }

    /* =========================================================
     * 阶段三：大满贯结束，按键功能恢复
     * ========================================================= */
    printf("\r\n========================================\r\n");
    printf("ALL TESTS COMPLETED SUCCESSFULLY!\r\n");
    printf("You can still press keys to toggle LEDs.\r\n");
    printf("========================================\r\n");

    /* 把按键切回中断模式，死循环里按啥亮啥 */
    app_key_init_exti(); 
    
    while (1) {
        delay_1ms(1000);
    }
}