#include "drv_at24c02.h"

/*!
    \brief      探测 AT24C02 是否在线
    \retval     0 在线，非0 不在线
*/
uint8_t at24c02_init(void)
{
    uint8_t dummy;
    return bsp_i2c_read_bytes(AT24C02_ADDR, 0x00, &dummy, 1);
}

/*!
    \brief      写一个字节
    \retval     0成功，非0失败
*/
uint8_t at24c02_write_byte(uint8_t mem_addr, uint8_t data)
{
    if (bsp_i2c_write_bytes(AT24C02_ADDR, mem_addr, &data, 1)) return 1;
    bsp_i2c_wait_standby(AT24C02_ADDR);
    return 0;
}

/*!
    \brief      读一个字节
    \retval     0成功，非0失败
*/
uint8_t at24c02_read_byte(uint8_t mem_addr, uint8_t *data)
{
    return bsp_i2c_read_bytes(AT24C02_ADDR, mem_addr, data, 1);
}

/*!
    \brief      按页写多个字节（内部自动分页）
    \retval     0成功，非0失败
*/
uint8_t at24c02_write_buffer(uint8_t mem_addr, uint8_t *buf, uint16_t len)
{
    uint16_t offset = 0;
    while (offset < len) {
        uint8_t  page_remain = AT24C02_PAGE_SIZE - (mem_addr % AT24C02_PAGE_SIZE);
        uint16_t chunk = (len - offset < page_remain) ? (len - offset) : page_remain;
        if (bsp_i2c_write_bytes(AT24C02_ADDR, mem_addr, buf + offset, chunk)) return 1;
        bsp_i2c_wait_standby(AT24C02_ADDR);
        mem_addr += chunk;
        offset   += chunk;
    }
    return 0;
}

/*!
    \brief      连续读多个字节
    \retval     0成功，非0失败
*/
uint8_t at24c02_read_buffer(uint8_t mem_addr, uint8_t *buf, uint16_t len)
{
    return bsp_i2c_read_bytes(AT24C02_ADDR, mem_addr, buf, len);
}

/*!
    \brief      全片写读一致性测试
    \note       按页写 0x00~0xFF，再整片读回比较
    \retval     0通过，非0失败
*/
uint8_t at24c02_test(void)
{
    uint8_t  wr_buf[AT24C02_PAGE_SIZE];
    uint8_t  rd_buf[AT24C02_PAGE_SIZE];
    uint16_t addr;

    for (addr = 0; addr < AT24C02_SIZE; addr += AT24C02_PAGE_SIZE) {
        for (uint8_t i = 0; i < AT24C02_PAGE_SIZE; i++) wr_buf[i] = (uint8_t)(addr + i);
        if (at24c02_write_buffer((uint8_t)addr, wr_buf, AT24C02_PAGE_SIZE)) return 1;
    }
    for (addr = 0; addr < AT24C02_SIZE; addr += AT24C02_PAGE_SIZE) {
        if (at24c02_read_buffer((uint8_t)addr, rd_buf, AT24C02_PAGE_SIZE)) return 1;
        for (uint8_t i = 0; i < AT24C02_PAGE_SIZE; i++)
            if (rd_buf[i] != (uint8_t)(addr + i)) return 1;
    }
    return 0;
}