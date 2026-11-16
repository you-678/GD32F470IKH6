#include "app_can.h"

void app_can_init(void)
{
    bsp_can_init(6); 
    printf("\r\n[CAN] Init OK. Press Tamper key to TX.\r\n");
}

void app_can_task(void)
{
    bsp_can_msg_t tx_msg;
    bsp_can_msg_t rx_msg;

    /* ---------------- 按键发送 ---------------- */
    /* 注意：如果你的按键读取函数不叫 bsp_key_state_get，请换成你 bsp_key.h 里的名字 */
    if(0 == bsp_key_state_get(KEY_TAMPER)) { 
        tx_msg.id = 0x7AB;
        tx_msg.len = 8;
        tx_msg.is_ext = 0;
        tx_msg.data[0] = 0xA0; tx_msg.data[1] = 0xA1;
        tx_msg.data[2] = 0xA2; tx_msg.data[3] = 0xA3;
        tx_msg.data[4] = 0xA4; tx_msg.data[5] = 0xA5;
        tx_msg.data[6] = 0xA6; tx_msg.data[7] = 0xA7;
        
        printf("\r\n[CAN] Transmit data:");
        for(uint8_t i = 0; i < tx_msg.len; i++) {
            printf(" %02x", tx_msg.data[i]);
        }
        
        bsp_can_send_msg(&tx_msg);

        while(0 == bsp_key_state_get(KEY_TAMPER)); 
    }

    /* ---------------- 接收处理 ---------------- */
    if(bsp_can_receive_msg(&rx_msg)) {
        /* 过滤：ID=0x7AB, 标准帧, 长度=8 */
        if((0x7AB == rx_msg.id) && (!rx_msg.is_ext) && (8 == rx_msg.len)) {
            
            /* 翻转 LED2 (请根据你的 bsp_led.h 实际函数名修改，可能是 bsp_led_toggle 或 gd_eval_led_toggle) */
            bsp_led_toggle(LED2); 
            
            printf("\r\n[CAN] Receive data:");
            for(uint8_t i = 0; i < rx_msg.len; i++) {
                printf(" %02x", rx_msg.data[i]);
            }
        }
    }
}