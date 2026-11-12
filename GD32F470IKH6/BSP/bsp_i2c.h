#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "gd32f4xx.h"

/*!
    \brief      I2C 工作模式枚举
      \arg        BSP_I2C_MODE_HW: 使用芯片内置硬件 I2C 外设
      \arg        BSP_I2C_MODE_SW: 使用 GPIO 模拟软件 I2C 时序
*/
typedef enum {
    BSP_I2C_MODE_HW = 0,
    BSP_I2C_MODE_SW
} bsp_i2c_mode_enum;

/*!
    \brief      初始化 I2C
    \param[in]  mode: 工作模式 (硬/软)
    \param[out] 无
    \retval     无
*/
void bsp_i2c_init(bsp_i2c_mode_enum mode);

/*!
    \brief      I2C 写入多个字节
    \param[in]  dev_addr: 从机地址 (如 0xA0)
    \param[in]  reg_addr: 寄存器/内部地址
    \param[in]  p_data: 数据首地址
    \param[in]  len: 写入长度
    \param[out] 无
    \retval     0成功，非0失败
*/
uint8_t bsp_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t len);

/*!
    \brief      I2C 读取多个字节
    \param[in]  dev_addr: 从机地址
    \param[in]  reg_addr: 寄存器/内部地址
    \param[out] p_data: 接收数据缓冲区
    \param[in]  len: 读取长度
    \retval     0成功，非0失败
*/
uint8_t bsp_i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint16_t len);

/*!
    \brief      I2C 等待 EEPROM 内部写入完成 (应答轮询)
    \param[in]  dev_addr: 从机地址
    \retval     无
*/
void bsp_i2c_wait_standby(uint8_t dev_addr);

#endif