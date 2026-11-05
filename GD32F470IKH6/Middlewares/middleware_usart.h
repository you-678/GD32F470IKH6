#ifndef __MIDDLEWARE_USART_H
#define __MIDDLEWARE_USART_H

#include "gd32f4xx.h"

#define BUFFER_SIZE  256U

extern uint8_t tx_buffer[BUFFER_SIZE];
extern uint8_t rx_buffer[BUFFER_SIZE];
extern volatile uint16_t tx_counter;
extern volatile uint16_t rx_counter;
extern uint32_t nbr_data_to_read;
extern uint32_t nbr_data_to_send;
extern volatile ErrStatus transfer_status;

void middleware_usart_init(uint32_t baudrate);
void middleware_usart_send_string(char *str);

void middleware_usart_reset(void);
ErrStatus middleware_usart_compare(uint8_t* src, uint8_t* dst, uint16_t length);

#endif
