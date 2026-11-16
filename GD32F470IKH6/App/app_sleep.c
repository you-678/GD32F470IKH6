#include "app_sleep.h"

void app_sleep_test(void)
{
    uint8_t rx_data[64];
    uint16_t rx_len = 0;
    uint8_t dummy[64];
    uint16_t dummy_len;

    printf("\r\n======================================\r\n");
    printf("[Sleep] Demo started.\r\n");
    
    /* 清空历史脏数据 */
    bsp_usart_get_rx_data(BSP_USART_COM0, dummy, &dummy_len); 

    printf("[Sleep] Entering sleep mode in 1 second...\r\n");
    delay_1ms(1000);

    bsp_led_off(LED1);
    bsp_led_off(LED2);
    bsp_led_off(LED3);

    printf("[Sleep] Sleeping now. Send any data from USART to wake up.\r\n");
    delay_1ms(100); 

    /* ★★★ 关键操作：进入睡眠前，关闭 SysTick 中断 ★★★ */
    /* 这样 WFI 就不会被 1ms 一次的滴答定时器叫醒了 */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk; 

    /* 1. 进入睡眠 */
    bsp_pmu_init();
    bsp_enter_sleep_mode();

    /* ========================================================
       程序停在这里，直到串口收到数据（USART0_IRQHandler 触发）
       ======================================================== */
    
    /* ★★★ 醒来后，立刻恢复 SysTick 中断，让 delay_1ms 继续能用 ★★★ */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; 

    printf("\r\n>>> [Wakeup] I am awake! <<<\r\n");

    /* 2. 读取唤醒数据 */
    if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_data, &rx_len)) {
        rx_data[rx_len] = '\0';
        printf("[Wakeup] Woken up by data: %s (Length: %d)\r\n", rx_data, rx_len);
    } else {
        printf("[Wakeup] Woken up, but NO data found!\r\n");
    }

    /* 3. 唤醒后闪灯 */
    printf("[Wakeup] Entering while(1) loop...\r\n");
    while(1) {
        bsp_led_toggle(LED1);
        bsp_led_toggle(LED2);
        bsp_led_toggle(LED3);
        delay_1ms(500);
    }
}