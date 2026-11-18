#include "app_usart.h"

void app_usart_task(void) {
    static uint8_t  line_buf[256];   /* 行累积缓冲，static 保证 DMA 回显期间有效 */
    static uint16_t line_len = 0;

    uint8_t  rx_data[256];
    uint16_t rx_len = 0;

    if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_data, &rx_len)) {
        for (uint16_t i = 0; i < rx_len; i++) {
            uint8_t ch = rx_data[i];

            if (ch == '\r' || ch == '\n') {
                if (line_len > 0) {
                    line_buf[line_len] = '\0';

                    if (strstr((char*)line_buf, "TRNG") != NULL) {
                        printf("\r\n[TRNG] random1 = %d, random2 = %d\r\n",
                               bsp_trng_random_range_get(0, 255),
                               bsp_trng_random_range_get(0, 255));
                    } else if (strstr((char*)line_buf, "BAUD9600") != NULL) {
                        printf("\r\nChanging baudrate to 9600...\r\n");
                        delay_1ms(50);
                        bsp_usart_set_baudrate(BSP_USART_COM0, 9600U);
                        printf("\r\n[Success] Baudrate is now 9600 bps.\r\n");
                    } else {
                        printf("\r\n[IRQ RX] Got: %s\r\n", line_buf);
                        bsp_usart_send_dma(BSP_USART_COM0, line_buf, line_len);
                    }
                    line_len = 0;
                }
            } else {
                if (line_len < sizeof(line_buf) - 1) {
                    line_buf[line_len++] = ch;
                }
            }
        }
        bsp_led_toggle(LED1);
    }
}