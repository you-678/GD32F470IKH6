#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "gd32f4xx.h"

/*!
    \brief      串口编号枚举
      \arg        BSP_USART_COM0: 串口 0 (USART0)
      \arg        BSP_USART_COMn: 串口总数 (用于静态数组映射)
*/
typedef enum { 
    BSP_USART_COM0 = 0, 
    BSP_USART_COMn      
} usart_typedef_enum;

/*!
    \brief      串口工作模式枚举
      \arg        BSP_USART_MODE_PRINTF: 仅发送模式 (用于 printf 打印)
      \arg        BSP_USART_MODE_IRQ: 中断收发模式 (带接收缓冲区)
*/
typedef enum { 
    BSP_USART_MODE_PRINTF = 0, 
    BSP_USART_MODE_IRQ         
} usartmode_typedef_enum;

/*!
    \brief      初始化串口硬件参数
    \param[in]  usart_num: 串口编号
      \arg        BSP_USART_COM0: 串口 0
    \param[in]  usart_mode: 串口工作模式
      \arg        BSP_USART_MODE_PRINTF: 仅发送模式
      \arg        BSP_USART_MODE_IRQ: 中断收发模式
    \param[in]  baudrate: 设定的波特率数值 (例如: 115200U)
    \param[out] 无
    \retval     无
*/
void bsp_usart_init(usart_typedef_enum usart_num, usartmode_typedef_enum usart_mode, uint32_t baudrate);

/*!
    \brief      在运行中途动态修改串口的波特率
    \param[in]  usart_num: 串口编号
      \arg        BSP_USART_COM0: 串口 0
    \param[in]  baudrate: 新的波特率数值 (例如: 9600U)
    \param[out] 无
    \retval     无
*/
void bsp_usart_set_baudrate(usart_typedef_enum usart_num, uint32_t baudrate);

/*!
    \brief      通过 DMA 方式发送一串数据
    \param[in]  usart_num: 串口编号
      \arg        BSP_USART_COM0: 串口 0
    \param[in]  data: 要发送的数据缓冲区首地址
    \param[in]  len: 要发送的数据长度(字节)
    \param[out] 无
    \retval     无
*/
void bsp_usart_send_dma(usart_typedef_enum usart_num, uint8_t *data, uint16_t len);

/*!
    \brief      获取串口中断接收到的数据
    \param[in]  usart_num: 串口编号
      \arg        BSP_USART_COM0: 串口 0
    \param[out] data: 用于存放读出数据的缓冲区指针
    \param[out] len: 用于返回实际读出的数据长度
    \retval     返回 1 表示有新数据，返回 0 表示暂无数据
*/
uint8_t bsp_usart_get_rx_data(usart_typedef_enum usart_num, uint8_t *data, uint16_t *len);

#endif