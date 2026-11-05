#ifndef __LED_APP_H
#define __LED_APP_H

#include "gd32f4xx.h"

void led_flow_task(void);

void app_led_flash(uint8_t times);
void app_led1_toggle(void);
void app_led2_toggle(void);
void app_led3_toggle(void);

#endif
