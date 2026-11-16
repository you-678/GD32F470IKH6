#ifndef __BSP_CLOCKOUT_H
#define __BSP_CLOCKOUT_H

#include "gd32f4xx.h"

/*  ±÷” ‰≥ˆ–Ú∫≈√∂æŸ */
typedef enum {
    CKOUT0_IRC16M_CKOUT1_SYSTEMCLOCK,
    CKOUT0_LXTAL_CKOUT1_PLLI2SR,
    CKOUT0_HXTAL_CKOUT1_HXTAL,
    CKOUT_COUNT
} clock_output_enum;

void bsp_clockout_init(void);
void bsp_clockout_select(uint8_t seq);

#endif /* __BSP_CLOCKOUT_H */