#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "gd32f4xx.h"

void bsp_timer_pwm_init(void);
void bsp_timer_pwm_set_duty(uint16_t duty);

#endif /* __BSP_TIMER_H */