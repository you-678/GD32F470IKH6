#ifndef __BSP_CAN_H
#define __BSP_CAN_H

#include "gd32f4xx.h"

/* 通用 CAN 报文结构体 */
typedef struct {
    uint32_t id;        /* 帧 ID */
    uint8_t  len;       /* 数据长度 0~8 */
    uint8_t  data[8];   /* 数据内容 */
    uint8_t  is_ext;    /* 0: 标准帧, 1: 扩展帧 */
} bsp_can_msg_t;

void    bsp_can_init(uint32_t baudrate_prescaler);
uint8_t bsp_can_send_msg(bsp_can_msg_t *msg);
uint8_t bsp_can_receive_msg(bsp_can_msg_t *msg);

#endif /* __BSP_CAN_H */