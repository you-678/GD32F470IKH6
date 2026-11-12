#include "bsp_i2c.h"

static bsp_i2c_mode_enum g_i2c_mode = BSP_I2C_MODE_HW;

/* ================== 软件 I2C 基础时序 ================== */
#define SW_SCL_H()  gpio_bit_set(BSP_I2C_GPIO_PORT, BSP_I2C_SCL_PIN)
#define SW_SCL_L()  gpio_bit_reset(BSP_I2C_GPIO_PORT, BSP_I2C_SCL_PIN)
#define SW_SDA_H()  gpio_bit_set(BSP_I2C_GPIO_PORT, BSP_I2C_SDA_PIN)
#define SW_SDA_L()  gpio_bit_reset(BSP_I2C_GPIO_PORT, BSP_I2C_SDA_PIN)
#define SW_SDA_READ() gpio_input_bit_get(BSP_I2C_GPIO_PORT, BSP_I2C_SDA_PIN)

static void sw_i2c_delay(void) {
    for (volatile int i = 0; i < 50; i++);
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
    SW_SDA_H();
    sw_i2c_delay();
    SW_SCL_H(); sw_i2c_delay();
    ack = SW_SDA_READ();
    SW_SCL_L(); sw_i2c_delay();
    return ack;
}

static uint8_t sw_i2c_read_byte(uint8_t ack) {
    uint8_t byte = 0;
    SW_SDA_H();
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        SW_SCL_H(); sw_i2c_delay();
        if (SW_SDA_READ()) byte |= 0x01;
        SW_SCL_L(); sw_i2c_delay();
    }
    if (ack == 0) SW_SDA_L(); else SW_SDA_H();
    sw_i2c_delay();
    SW_SCL_H(); sw_i2c_delay();
    SW_SCL_L(); sw_i2c_delay();
    SW_SDA_H();
    return byte;
}

/* ================== 硬件 I2C 基础时序 ================== */

/*!
    \brief      第一次 START：等待总线空闲，再发 START
*/
static void hw_i2c_start(void) {
    uint32_t t = 0xFFFFF;
    while (i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_I2CBSY)) {
        if (--t == 0) { printf("[I2C] START: BUSY timeout\r\n"); return; }
    }
    i2c_start_on_bus(BSP_I2C_PERIPH);
    t = 0xFFFFF;
    while (!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_SBSEND)) {
        if (--t == 0) { printf("[I2C] START: SBSEND timeout\r\n"); return; }
    }
}

/*!
    \brief      重复 START：总线已经忙，不能等 I2CBSY 清零
*/
static void hw_i2c_start_repeat(void) {
    i2c_start_on_bus(BSP_I2C_PERIPH);
    uint32_t t = 0xFFFFF;
    while (!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_SBSEND)) {
        if (--t == 0) { printf("[I2C] START(repeat): SBSEND timeout\r\n"); return; }
    }
}

/*!
    \brief      产生停止位（带超时）
*/
static void hw_i2c_stop(void) {
    i2c_stop_on_bus(BSP_I2C_PERIPH);
    uint32_t t = 0xFFFFF;
    while ((I2C_CTL0(BSP_I2C_PERIPH) & I2C_CTL0_STOP) && --t);
}

/*!
    \brief      发送 7bit 地址 + R/W，必须等 ADDSEND 并清除
    \retval     0 成功，1 失败
*/
static uint8_t hw_i2c_send_addr(uint8_t addr) {
    uint32_t t = 0xFFFFF;
    while ((I2C_STAT0(BSP_I2C_PERIPH) & I2C_FLAG_TBE) == 0) {
        if (--t == 0) { printf("[I2C] ADDR: TBE timeout\r\n"); return 1; }
    }
    i2c_data_transmit(BSP_I2C_PERIPH, addr);

    t = 0xFFFFF;
    while (!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_ADDSEND)) {
        if (--t == 0) {
            printf("[I2C] ADDR: ADDSEND timeout, STAT0=0x%08X, addr=0x%02X\r\n",
                   (unsigned)I2C_STAT0(BSP_I2C_PERIPH), addr);
            return 1;
        }
    }
    (void)I2C_STAT0(BSP_I2C_PERIPH);
    (void)I2C_STAT1(BSP_I2C_PERIPH);

    if (i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_AERR)) {
        i2c_flag_clear(BSP_I2C_PERIPH, I2C_FLAG_AERR);
        return 1;
    }
    return 0;
}

/*!
    \brief      发送一个数据字节（带超时）
    \retval     0 成功，1 失败
*/
static uint8_t hw_i2c_send_byte(uint8_t byte) {
    uint32_t t = 0xFFFFF;
    while (SET != i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_TBE)) {
        if (--t == 0) return 1;
    }
    i2c_data_transmit(BSP_I2C_PERIPH, byte);
    t = 0xFFFFF;
    while (!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_BTC)) {
        if (--t == 0) return 1;
    }
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
*/
void bsp_i2c_init(bsp_i2c_mode_enum mode) {
    g_i2c_mode = mode;
    rcu_periph_clock_enable(BSP_I2C_GPIO_RCU);

    if (mode == BSP_I2C_MODE_HW) {
        rcu_periph_clock_enable(BSP_I2C_RCU);
        i2c_deinit(BSP_I2C_PERIPH);

        gpio_af_set(BSP_I2C_GPIO_PORT, BSP_I2C_AF, BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        gpio_mode_set(BSP_I2C_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP,
                      BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        gpio_output_options_set(BSP_I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
                                BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);

        i2c_clock_config(BSP_I2C_PERIPH, 100000, I2C_DTCY_2);
        i2c_mode_addr_config(BSP_I2C_PERIPH, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x00);
        i2c_enable(BSP_I2C_PERIPH);
        i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_ENABLE);

        printf("I2CBSY after init = %d\r\n",
               i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_I2CBSY) ? 1 : 0);
    } else {
        gpio_mode_set(BSP_I2C_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,
                      BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        gpio_output_options_set(BSP_I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
                                BSP_I2C_SCL_PIN | BSP_I2C_SDA_PIN);
        SW_SCL_H(); SW_SDA_H();
    }
}

/*!
    \brief      I2C 写入多个字节
*/
uint8_t bsp_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t len) {
    if (g_i2c_mode == BSP_I2C_MODE_HW) {
        hw_i2c_start();
        if (hw_i2c_send_addr(dev_addr & 0xFE)) { hw_i2c_stop(); return 1; }
        if (hw_i2c_send_byte(reg_addr))        { hw_i2c_stop(); return 1; }
        for (uint16_t i = 0; i < len; i++) {
            if (hw_i2c_send_byte(p_data[i]))   { hw_i2c_stop(); return 1; }
        }
        hw_i2c_stop();
    } else {
        sw_i2c_start();
        if (sw_i2c_send_byte(dev_addr & 0xFE)) { sw_i2c_stop(); return 1; }
        if (sw_i2c_send_byte(reg_addr))        { sw_i2c_stop(); return 1; }
        for (uint16_t i = 0; i < len; i++) {
            if (sw_i2c_send_byte(p_data[i]))   { sw_i2c_stop(); return 1; }
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
        if (hw_i2c_send_addr(dev_addr & 0xFE)) { hw_i2c_stop(); return 1; }
        if (hw_i2c_send_byte(reg_addr))        { hw_i2c_stop(); return 1; }

        hw_i2c_start_repeat();
        if (hw_i2c_send_addr(dev_addr | 0x01)) { hw_i2c_stop(); return 1; }

        for (uint16_t i = 0; i < len; i++) {
            if (i == len - 1) {
                i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_DISABLE);
            }
            uint32_t t = 0xFFFFF;
            while (!i2c_flag_get(BSP_I2C_PERIPH, I2C_FLAG_RBNE)) {
                if (--t == 0) {
                    i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_ENABLE);
                    hw_i2c_stop();
                    return 1;
                }
            }
            p_data[i] = i2c_data_receive(BSP_I2C_PERIPH);
        }
        i2c_ack_config(BSP_I2C_PERIPH, I2C_ACK_ENABLE);
        hw_i2c_stop();
    } else {
        sw_i2c_start();
        if (sw_i2c_send_byte(dev_addr & 0xFE)) { sw_i2c_stop(); return 1; }
        if (sw_i2c_send_byte(reg_addr))        { sw_i2c_stop(); return 1; }

        sw_i2c_start();
        if (sw_i2c_send_byte(dev_addr | 0x01)) { sw_i2c_stop(); return 1; }

        for (uint16_t i = 0; i < len; i++) {
            p_data[i] = sw_i2c_read_byte(i == len - 1 ? 1 : 0);
        }
        sw_i2c_stop();
    }
    return 0;
}

/*!
    \brief      I2C 等待 EEPROM 内部写入完成
*/
void bsp_i2c_wait_standby(uint8_t dev_addr) {
    uint32_t timeout = 0xFFFFFF;
    while (timeout--) {
        if (bsp_i2c_write_bytes(dev_addr, 0x00, 0, 0) == 0) {
            break;
        }
    }
}