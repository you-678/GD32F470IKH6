#ifndef DCI_OV2640_H
#define DCI_OV2640_H

#include "gd32f4xx.h"

#define DCI_TIMEOUT        20000

typedef struct
{
  uint8_t manufacturer_id1;
  uint8_t manufacturer_id2;
  uint8_t version;
  uint8_t pid; 
}ov2640_id_struct; 

/* ov2640 registers definition */
#define OV2640_PID        0x0A
#define OV2640_VER        0x0B
#define OV2640_MIDH       0x1C 
#define OV2640_MIDL       0x1D 

/* 只保留与摄像头芯片协议相关的函数！ */
/* dci_config 和 ckout0_init 已经被踢到 BSP 层了，这里删掉 */
uint8_t dci_ov2640_init(void);
uint8_t dci_ov2640_id_read(ov2640_id_struct* ov2640id);
uint8_t ov2640_outsize_set(uint16_t width,uint16_t height);

#endif /* DCI_OV2640_H */