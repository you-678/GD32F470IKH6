#include "bsp_can.h"

/* 中断接收缓存 */
static can_receive_message_struct s_rx_hw_msg;
static volatile uint8_t s_rx_flag = 0;

/*!
    \brief      CAN0 接收中断服务函数（必须写在 BSP 层，并覆盖启动文件弱函数）
*/
void CAN0_RX0_IRQHandler(void)
{
    can_message_receive(CAN0_PERIPH, CAN_FIFO0, &s_rx_hw_msg);
    s_rx_flag = 1;
}

/*!
    \brief      初始化 CAN 硬件
    \param[in]  baudrate_prescaler: 分频值 (1Mbps 通常为 6，取决于 APB1 时钟)
*/
void bsp_can_init(uint32_t baudrate_prescaler)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

    rcu_periph_clock_enable(CAN0_CLK);
    rcu_periph_clock_enable(CAN0_GPIO_CLK);
    
    /* TX */
    gpio_output_options_set(CAN0_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CAN0_TX_PIN);
    gpio_mode_set(CAN0_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CAN0_TX_PIN);
    gpio_af_set(CAN0_GPIO_PORT, CAN0_AF, CAN0_TX_PIN);
    
    /* RX */
    gpio_output_options_set(CAN0_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CAN0_RX_PIN);
    gpio_mode_set(CAN0_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, CAN0_RX_PIN);
    gpio_af_set(CAN0_GPIO_PORT, CAN0_AF, CAN0_RX_PIN);

    /* 中断配置 */
    nvic_irq_enable(CAN0_RX_IRQn, 0, 0);

    /* CAN 参数配置 */
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_deinit(CAN0_PERIPH);
    
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = ENABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_LOOPBACK_MODE;//CAN_NORMAL_MODE;
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_7TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_2TQ;
    can_parameter.prescaler = baudrate_prescaler;
    can_init(CAN0_PERIPH, &can_parameter);

    /* 过滤器配置（接收所有） */
    can_struct_para_init(CAN_FILTER_STRUCT, &can_filter);
    can_filter.filter_number = 0;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;
    can_filter_init(&can_filter);

    /* 使能 FIFO0 接收中断 */
    can_interrupt_enable(CAN0_PERIPH, CAN_INTEN_RFNEIE0);
}

/*!
    \brief      发送报文
    \retval     1:发送成功 0:发送超时
*/
uint8_t bsp_can_send_msg(bsp_can_msg_t *msg)
{
    can_trasnmit_message_struct tx_msg;
    uint8_t mailbox;
    uint32_t timeout = 0xFFFF;

    tx_msg.tx_sfid = msg->id;
    tx_msg.tx_efid = 0x00;
    tx_msg.tx_ft = CAN_FT_DATA;
    tx_msg.tx_ff = msg->is_ext ? CAN_FF_EXTENDED : CAN_FF_STANDARD;
    tx_msg.tx_dlen = msg->len;
    for(uint8_t i = 0; i < msg->len; i++) {
        tx_msg.tx_data[i] = msg->data[i];
    }

    mailbox = can_message_transmit(CAN0_PERIPH, &tx_msg);
    
    while((CAN_TRANSMIT_OK != can_transmit_states(CAN0_PERIPH, mailbox)) && (0 != timeout)){
        timeout--;
    }
    return (timeout == 0) ? 0 : 1;
}

/*!
    \brief      读取接收到的报文
    \retval     1:有新数据 0:无数据
*/
uint8_t bsp_can_receive_msg(bsp_can_msg_t *msg)
{
    if(s_rx_flag) {
        msg->id = s_rx_hw_msg.rx_sfid;
        msg->len = s_rx_hw_msg.rx_dlen;
        msg->is_ext = (s_rx_hw_msg.rx_ff == CAN_FF_EXTENDED) ? 1 : 0;
        for(uint8_t i = 0; i < msg->len; i++) {
            msg->data[i] = s_rx_hw_msg.rx_data[i];
        }
        s_rx_flag = 0;
        return 1;
    }
    return 0;
}