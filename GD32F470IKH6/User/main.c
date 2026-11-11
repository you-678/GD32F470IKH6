#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

/*!
    \brief      系统主函数 - 串口终极测试 (先测 Printf，再测 IRQ，最后测 DMA)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    uint8_t rx_buf[256];
    uint16_t rx_len = 0;

    /* 1. 基础环境初始化 */
    systick_config();
    bsp_led_init(LED1);

    /* =========================================================
     * 阶段一：纯 Printf 测试 (持续 5 秒)
     * ========================================================= */
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("STAGE 1: Printf Test Started (5 seconds)\r\n");
    printf("You should see heartbeats below.\r\n");
    printf("========================================\r\n");

    /* 循环 50 次，每次 100ms，总计 5 秒 */
    for (int i = 0; i < 50; i++) {
        printf("[Printf] Heartbeat %d\r\n", i);
        delay_1ms(100);
    }

    /* =========================================================
     * 阶段二：中断接收 (IRQ) + 普通回显 (持续 10 秒)
     * ========================================================= */
    /* 重新初始化串口，开启中断接收 */
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("STAGE 2: IRQ RX & Printf Echo (10 seconds)\r\n");
    printf("Please send some data to test IRQ.\r\n");
    printf("========================================\r\n");

    /* 循环 1000 次，每次 10ms，总计 10 秒 */
    for (int i = 0; i < 1000; i++) {
        /* 查询是否收到数据 */
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0'; /* 添加结束符 */
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }

    /* =========================================================
     * 阶段三：中断接收 (IRQ) + DMA 发送回显 (无限循环)
     * ========================================================= */
    printf("\r\n========================================\r\n");
    printf("STAGE 3: IRQ RX & DMA Echo (Infinite loop)\r\n");
    printf("Please send some data to test DMA echo.\r\n");
    printf("========================================\r\n");

    while(1) {
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0';
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            
            /* 调用底层 DMA 发送函数，把收到的数据发回去 */
            bsp_usart_send_dma(BSP_USART_COM0, rx_buf, rx_len);
            
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }
}