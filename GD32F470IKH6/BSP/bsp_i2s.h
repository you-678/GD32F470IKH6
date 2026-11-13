#ifndef __BSP_I2S_H
#define __BSP_I2S_H

#include "gd32f4xx.h"

typedef enum {
    BSP_I2S_BUS_AUDIO = 0,
    BSP_I2S_BUS_MAX
} bsp_i2s_bus_t;

typedef enum {
    BSP_I2S_MODE_HW = 0,
    BSP_I2S_MODE_SW,
} bsp_i2s_mode_t;

typedef uint16_t (*bsp_i2s_tx_cb_t)(void *ctx);

void           bsp_i2s_init_mode(bsp_i2s_bus_t bus, bsp_i2s_mode_t mode);
bsp_i2s_mode_t bsp_i2s_get_mode(bsp_i2s_bus_t bus);

void bsp_i2s_start(bsp_i2s_bus_t bus, uint32_t sample_rate_hz,
                   bsp_i2s_tx_cb_t cb, void *ctx, uint32_t num_halfwords);

void bsp_i2s_stop(bsp_i2s_bus_t bus);
int  bsp_i2s_is_done(bsp_i2s_bus_t bus);
void bsp_i2s_irq_handler(bsp_i2s_bus_t bus);

#endif