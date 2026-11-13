#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "gd32f4xx.h"

/* 默认模式：0 = 硬件 SPI5，1 = 软件 SPI */
#ifndef BSP_SPI_FLASH_USE_SW
#define BSP_SPI_FLASH_USE_SW     0
#endif

/* 软件 SPI 半周期延时（NOP 次数） */
#ifndef BSP_SPI_SW_DELAY_LOOPS
#define BSP_SPI_SW_DELAY_LOOPS   4
#endif

typedef enum {
    BSP_SPI_BUS_FLASH = 0,
    BSP_SPI_BUS_LCD,
    BSP_SPI_BUS_MAX
} bsp_spi_bus_t;

typedef enum {
    BSP_SPI_MODE_HW = 0,
    BSP_SPI_MODE_SW,
} bsp_spi_mode_t;

void           bsp_spi_init(bsp_spi_bus_t bus);
void           bsp_spi_init_mode(bsp_spi_bus_t bus, bsp_spi_mode_t mode);
bsp_spi_mode_t bsp_spi_get_mode(bsp_spi_bus_t bus);

uint8_t bsp_spi_transfer(bsp_spi_bus_t bus, uint8_t tx_byte);
void    bsp_spi_transfer_buf(bsp_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len);

void    bsp_spi_cs_low(bsp_spi_bus_t bus);
void    bsp_spi_cs_high(bsp_spi_bus_t bus);

void    bsp_spi_quad_enter_read(bsp_spi_bus_t bus);
void    bsp_spi_quad_enter_write(bsp_spi_bus_t bus);
void    bsp_spi_quad_exit(bsp_spi_bus_t bus);

int     bsp_spi_supports_quad(bsp_spi_bus_t bus);

#endif /* __BSP_SPI_H */