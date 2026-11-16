#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include "gd32f4xx.h"

#define RTC_BKP_MAGIC   0x32F0

void bsp_rtc_init(void);
void bsp_rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second);
void bsp_rtc_show_time(void);
uint8_t bsp_rtc_is_configured(void);
void bsp_rtc_mark_configured(void);

#endif /* __BSP_RTC_H */