#include "bsp_sccb.h"

void bsp_sccb_init(void) {
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(SCCB_CLK);
    gpio_af_set(SCCB_SCL_PORT, SCCB_AF, SCCB_SCL_PIN);
    gpio_af_set(SCCB_SDA_PORT, SCCB_AF, SCCB_SDA_PIN);
    gpio_mode_set(SCCB_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SCCB_SCL_PIN);
    gpio_output_options_set(SCCB_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, SCCB_SCL_PIN);
    gpio_mode_set(SCCB_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SCCB_SDA_PIN);
    gpio_output_options_set(SCCB_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, SCCB_SDA_PIN);
    
    i2c_deinit(SCCB_PERIPH);
    i2c_clock_config(SCCB_PERIPH, 100000, I2C_DTCY_2);
    i2c_mode_addr_config(SCCB_PERIPH, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0xFE);
    i2c_enable(SCCB_PERIPH);
    i2c_ack_config(SCCB_PERIPH, I2C_ACK_ENABLE);
}

uint8_t bsp_sccb_write(uint8_t reg, uint8_t data) {
    volatile uint32_t timeout = 20000000;
    /* 原版这里是无超时死循环：摄像头没应答时 I2CBSY 永不清除，程序卡死在红色。
       加超时保护：总线忙超过一定时间直接返回失败，不再死等 */
    while(i2c_flag_get(SCCB_PERIPH, I2C_FLAG_I2CBSY)){ if ((timeout--) == 0) return 0xFF; }
    i2c_start_on_bus(SCCB_PERIPH);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_SBSEND)){ if ((timeout--) == 0) return 0xFF; }
    i2c_master_addressing(SCCB_PERIPH, OV2640_DEVICE_WRITE_ADDR, I2C_TRANSMITTER);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_ADDSEND)){ if ((timeout--) == 0) return 0xFF; }
    i2c_flag_clear(SCCB_PERIPH, I2C_FLAG_ADDSEND);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_TBE)){ if ((timeout--) == 0) return 0xFF; }
    i2c_data_transmit(SCCB_PERIPH, reg);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_BTC)){ if ((timeout--) == 0) return 0xFF; }
    i2c_data_transmit(SCCB_PERIPH, data);    
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_TBE)){ if ((timeout--) == 0) return 0xFF; }  
    i2c_stop_on_bus(SCCB_PERIPH);
    return 0;
}

uint8_t bsp_sccb_read(uint8_t reg, uint8_t *data) {
    volatile uint32_t timeout = 20000000;
    I2C_STAT0(SCCB_PERIPH) |= (uint16_t)0x0400;
    i2c_start_on_bus(SCCB_PERIPH);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_SBSEND)){ if ((timeout--) == 0) return 0xFF; } 
    i2c_master_addressing(SCCB_PERIPH, OV2640_DEVICE_READ_ADDR, I2C_TRANSMITTER);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_ADDSEND)){ if ((timeout--) == 0) return 0xFF; }
    i2c_flag_clear(SCCB_PERIPH, I2C_FLAG_ADDSEND);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_TBE)){ if ((timeout--) == 0) return 0xFF; }
    i2c_data_transmit(SCCB_PERIPH, reg);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_BTC)){ if ((timeout--) == 0) return 0xFF; } 
    I2C_STAT0(SCCB_PERIPH) |= (uint16_t)0x0400;
    i2c_stop_on_bus(SCCB_PERIPH);
    i2c_start_on_bus(SCCB_PERIPH);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_SBSEND)){ if ((timeout--) == 0) return 0xFF; } 
    i2c_master_addressing(SCCB_PERIPH, OV2640_DEVICE_WRITE_ADDR, I2C_RECEIVER);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_ADDSEND)){ if ((timeout--) == 0) return 0xFF; }
    i2c_flag_clear(SCCB_PERIPH, I2C_FLAG_ADDSEND); 
    i2c_ack_config(SCCB_PERIPH, I2C_ACK_DISABLE);
    timeout = 20000000; while(!i2c_flag_get(SCCB_PERIPH, I2C_FLAG_RBNE)){ if ((timeout--) == 0) return 0xFF; } 
    i2c_stop_on_bus(SCCB_PERIPH); 
    *data = i2c_data_receive(SCCB_PERIPH);
    I2C_STAT0(SCCB_PERIPH) |= (uint16_t)0x0400;
    return 0;
}