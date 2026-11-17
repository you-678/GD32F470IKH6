#ifndef __BSP_IPA_H
#define __BSP_IPA_H

#include "gd32f4xx.h"

void bsp_ipa_init_config(uint32_t src_addr, uint32_t dst_addr, uint16_t width, uint16_t height);
void bsp_ipa_start(void);

#endif /* __BSP_IPA_H */