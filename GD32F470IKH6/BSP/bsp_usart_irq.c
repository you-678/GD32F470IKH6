#include "bsp_usart_irq.h"

//// 开启NVIC + 串口收发中断
//void bsp_usart_irq_enable(void)
//{
//    nvic_irq_enable(BSP_USART_IRQn, 0U, 0U);
//    usart_interrupt_enable(BSP_USART_COM0, USART_INT_RBNE);
//    usart_interrupt_enable(BSP_USART_COM0, USART_INT_TBE);
//}

//void bsp_usart_irq_disable_rx(void)
//{
//    usart_interrupt_disable(BSP_USART_COM0, USART_INT_RBNE);
//}

//void bsp_usart_irq_disable_tx(void)
//{
//    usart_interrupt_disable(BSP_USART_COM0, USART_INT_TBE);
//}
