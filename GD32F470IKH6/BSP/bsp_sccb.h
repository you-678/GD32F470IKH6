#ifndef __BSP_SCCB_H
#define __BSP_SCCB_H

#include "gd32f4xx.h"

void    bsp_sccb_init(void);
uint8_t bsp_sccb_write(uint8_t reg, uint8_t data);
uint8_t bsp_sccb_read(uint8_t reg, uint8_t *data);

#endif