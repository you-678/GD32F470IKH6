#ifndef __DRV_AT24C02_H
#define __DRV_AT24C02_H

#include "gd32f4xx.h"

#define AT24C02_ADDR        BSP_EEPROM_ADDR
#define AT24C02_PAGE_SIZE   BSP_EEPROM_PAGE_SIZE
#define AT24C02_SIZE        256

/*!
    \brief      探测 AT24C02 是否在线
    \retval     0 在线，非0 不在线
*/
uint8_t at24c02_init(void);

/*!
    \brief      写一个字节
    \param[in]  mem_addr: 内部存储地址
    \param[in]  data:     待写数据
    \retval     0成功，非0失败
*/
uint8_t at24c02_write_byte(uint8_t mem_addr, uint8_t data);

/*!
    \brief      读一个字节
    \param[in]  mem_addr: 内部存储地址
    \param[out] data:     读出数据
    \retval     0成功，非0失败
*/
uint8_t at24c02_read_byte(uint8_t mem_addr, uint8_t *data);

/*!
    \brief      按页写多个字节（内部自动分页）
    \param[in]  mem_addr: 起始内部地址
    \param[in]  buf:      数据首地址
    \param[in]  len:      写入长度
    \retval     0成功，非0失败
*/
uint8_t at24c02_write_buffer(uint8_t mem_addr, uint8_t *buf, uint16_t len);

/*!
    \brief      连续读多个字节
    \param[in]  mem_addr: 起始内部地址
    \param[out] buf:      接收缓冲区
    \param[in]  len:      读取长度
    \retval     0成功，非0失败
*/
uint8_t at24c02_read_buffer(uint8_t mem_addr, uint8_t *buf, uint16_t len);

/*!
    \brief      全片写读一致性测试
    \retval     0通过，非0失败
*/
uint8_t at24c02_test(void);

#endif