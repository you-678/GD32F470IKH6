#ifndef __BSP_TLI_H
#define __BSP_TLI_H

#include "gd32f4xx.h"

void bsp_tli_init(void);
void bsp_tli_blend_layer1_init(uint32_t mem_addr, uint16_t width, uint16_t height);

#endif /* __BSP_TLI_H */