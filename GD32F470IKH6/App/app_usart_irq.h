#ifndef __APP_USART_H
#define __APP_USART_H

#include "middleware_usart.h"
#include "bsp_key.h"
#include "bsp_led.h"
#include "systick.h"

void app_usart_init(void);
void app_usart_task(void);

#endif
