#include "app_usart.h"

/*!
    \brief      串口综合应用任务
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_usart_task(void) {
    uint8_t rx_data[256]; 
    uint16_t rx_len = 0;

    if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_data, &rx_len)) {
        rx_data[rx_len] = '\0'; /* 添加字符串结束符 */
        
        if (strstr((char*)rx_data, "BAUD9600") != NULL) {
            printf("\r\nChanging baudrate to 9600...\r\n");
            delay_1ms(50);
            bsp_usart_set_baudrate(BSP_USART_COM0, 9600U);
            printf("\r\n[Success] Baudrate is now 9600 bps.\r\n");
        } else {
            printf("\r\n[IRQ RX] Got: %s\r\n", rx_data);
            bsp_usart_send_dma(BSP_USART_COM0, rx_data, rx_len); /* DMA 回显 */
        }
        
        bsp_led_toggle(LED1); /* 收到数据翻转LED1 */
    }
}