#include "bsp_i2c.h"

static bsp_i2c_mode_enum g_i2c_mode = BSP_I2C_MODE_HW;

/* ================== 软件 I2C 基础时序 ================== */
#define SW_SCL_H()  gpio_bit_set(BSP_I2C_GPIO_PORT, BSP_I2C_SCL_PIN)
#define SW_SCL_L()  gpio_bit_reset(BSP_I2C_GPIO_PORT, BSP_I2C_SCL_PIN)
#define SW_SDA_H()  gpio_bit_set(BSP_I2C_GPIO_PORT, BSP_I2C_SDA_PIN)
#define SW_SDA_L()  gpio_bit_reset(BSP_I2C_GPIO_PORT, BSP_I2C_SDA_PIN)
#define SW_SDA_READ() gpio_input_bit_get(BSP_I2C_GPIO_PORT, BSP_I2C_SDA_PIN)

static void sw_i2c_delay(void) {
    for (volatile int i = 0; i < 50; i++); /* 粗略延时，约几微秒 */
}

static void sw_i2c_start(void) {
    SW_SDA_H(); SW_SCL_H(); sw_i2c_delay();
    SW_SDA_L(); sw_i2c_delay();
    SW_SCL_L(); sw_i2c_delay();
}

static void sw_i2c_stop(void) {
    SW_SCL_L(); SW_SDA_L(); sw_i2c_delay();
    SW_SCL_H(); sw_i2c_delay();
    SW_SDA_H(); sw_i2c_delay();
}

static uint8_t sw_i2c_send_byte(uint8_t byte) {
    uint8_t ack;
    for (uint8_t i = 0; i < 8; i++) {
        if (byte & 0x80) SW_SDA_H(); else SW_SDA_L();
        byte <<= 1;
        sw_i2c_delay();
        SW_SCL_H(); sw_i2c_delay();
        SW_SCL_L(); sw_i2c_delay();
    }
    SW_SDA_H(); /* 释放SDA等待ACK */
    sw_i2c_delay();
    SW_SCL_H(); sw_i2c_delay();
    ack = SW_SDA_READ(); /* 0表示收到ACK */
    SW_SCL_L(); sw_i2c_delay();
    return ack;
}

static uint8_t sw_i2c_read_byte(uint8_t ack) {
    uint8_t byte = 0;
    SW_SDA_H(); /* 释放SDA */
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        SW_SCL_H(); sw_i2c_delay();
        if (SW_SDA_READ()) byte |= 0x01;
        SW_SCL_L(); sw_i2c_delay();
    }
    if (ack == 0) SW_SDA_L(); else SW_SDA_H(); /* 发送ACK或NACK */
    sw_i2c_delay();
    SW_SCL_H(); sw_i2c_delay();
    SW_SCL_L(); sw_i2c_delay();
    SW_SDA_H(); /* 释放SDA */
    return byte;
}

/* ================== 硬件 I2C 基础时序 ================== */
static void hw_i2c_start(void) {
    while(i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_I2CBSY));
    i2c_start_on_bus(BSP_I2C_PERIPH);
    while(!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_SBSEND));
}

static void hw_i2c_stop(void) {
    i2c_stop_on_bus(BSP_I2C_PERIPH);
    while(I2C_CTL0(BSP_I2C_PERIPH) & I2C_CTL0_STOP);
}

static uint8_t hw_i2c_send_byte(uint8_t byte) {
    while(SET != i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_TBE));
    i2c_data_transmit(BSP_I2C_PERIPH, byte);
    while(!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_BTC));
    if (i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_AERR)) {
        i2c_flag_clear(BSP_I2C_PERIPH, I2C_FLAG_AERR);
        return 1;
    }
    return 0;
}

/* ================== 对外统一接口 ================== */

/*!
    \brief      初始化 I2C
    \param[in]  mode: 工作模式
    \param[out] 无
    \retval     无
*/
void bsp_i2c_init(bsp_i2c_mode_enum mode) {
    g_i2c_mode = mode;
    rcu_periph_clock_enable(BSP_I2C_GPIO_RCU);

    if (mode == BSP_I2C_MODE_HW) {
        rcu_periph_clock_enable(BSP_I2C_RCU);
        gpio_af_set(BSP_I2C_GPIO_PORT, BSP_I2C_AF, BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        gpio_mode_set(BSP_I2C_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        gpio_output_options_set(BSP_I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);

        i2c_clock_config(BSP_I2C_PERIPH, 400000, I2C_DTCY_2);
        i2c_mode_addr_config(BSP_I2C_PERIPH, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
        i2c_enable(BSP_I2C_PERIPH);
        i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_ENABLE);
    } else {
        /* 软件 I2C 开漏输出模式 */
        gpio_mode_set(BSP_I2C_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        gpio_output_options_set(BSP_I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        SW_SCL_H(); SW_SDA_H();
    }
}

/*!
    \brief      I2C 写入多个字节
*/
uint8_t bsp_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t len) {
    if (g_i2c_mode == BSP_I2C_MODE_HW) {
        hw_i2c_start();
        if (hw_i2c_send_byte(dev_addr & 0xFE)) { hw_i2c_stop(); return 1; } /* 写 */
        if (hw_i2c_send_byte(reg_addr)) { hw_i2c_stop(); return 1; }
        for (uint16_t i = 0; i < len; i++) {
            if (hw_i2c_send_byte(p_data[i])) { hw_i2c_stop(); return 1; }
        }
        hw_i2c_stop();
    } else {
        sw_i2c_start();
        if (sw_i2c_send_byte(dev_addr & 0xFE)) { sw_i2c_stop(); return 1; }
        if (sw_i2c_send_byte(reg_addr)) { sw_i2c_stop(); return 1; }
        for (uint16_t i = 0; i < len; i++) {
            if (sw_i2c_send_byte(p_data[i])) { sw_i2c_stop(); return 1; }
        }
        sw_i2c_stop();
    }
    return 0;
}

/*!
    \brief      I2C 读取多个字节
*/
uint8_t bsp_i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t len) {
    if (len == 0) return 1;
    
    if (g_i2c_mode == BSP_I2C_MODE_HW) {
        hw_i2c_start();
        if (hw_i2c_send_byte(dev_addr & 0xFE)) { hw_i2c_stop(); return 1; } /* 写地址 */
        if (hw_i2c_send_byte(reg_addr)) { hw_i2c_stop(); return 1; }
        
        hw_i2c_start(); /* 重复起始 */
        if (hw_i2c_send_byte(dev_addr | 0x01)) { hw_i2c_stop(); return 1; } /* 读地址 */
        
        for (uint16_t i = 0; i < len; i++) {
            if (i == len - 1) {
                i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_DISABLE); /* 最后一个字节 NACK */
            }
            while(!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_RBNE));
            p_data[i] = i2c_data_receive(BSP_I2C_PERIPH);
        }
        i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_ENABLE);
        hw_i2c_stop();
    } else {
        sw_i2c_start();
        if (sw_i2c_send_byte(dev_addr & 0xFE)) { sw_i2c_stop(); return 1; }
        if (sw_i2c_send_byte(reg_addr)) { sw_i2c_stop(); return 1; }
        
        sw_i2c_start();
        if (sw_i2c_send_byte(dev_addr | 0x01)) { sw_i2c_stop(); return 1; }
        
        for (uint16_t i = 0; i < len; i++) {
            p_data[i] = sw_i2c_read_byte(i == len - 1 ? 1 : 0); /* 最后一个字节 NACK */
        }
        sw_i2c_stop();
    }
    return 0;
}

/*!
    \brief      I2C 等待 EEPROM 内部写入完成
*/
void bsp_i2c_wait_standby(uint8_t dev_addr) {
    uint32_t timeout = 0xFFFFF;
    while(timeout--) {
        if (bsp_i2c_write_bytes(dev_addr, 0x00, 0, 0) == 0) {
            break; /* 收到 ACK 说明写完了 */
        }
    }
}