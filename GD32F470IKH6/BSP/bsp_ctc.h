#ifndef __BSP_CTC_H
#define __BSP_CTC_H

#include "gd32f4xx.h"

void bsp_ctc_init(void);
uint8_t bsp_ctc_is_trim_ok(void);

#endif /* __BSP_CTC_H */