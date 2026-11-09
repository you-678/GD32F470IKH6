#ifndef __LED_APP_H
#define __LED_APP_H

#include "gd32f4xx.h"

void app_led_all_on(void);
void app_led_all_off(void);
void app_led_all_toggle(void);

void app_led_flow_task(void);
void app_led_flash(uint8_t times);

#endif
