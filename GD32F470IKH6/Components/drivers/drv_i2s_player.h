#ifndef __DRV_I2S_PLAYER_H
#define __DRV_I2S_PLAYER_H

#include "gd32f4xx.h"

typedef struct {
    bsp_i2s_bus_t  bus;
    drv_wav_info_t wav;
    drv_wav_iter_t iter;
} drv_i2s_player_t;

void drv_i2s_player_init(drv_i2s_player_t *p, bsp_i2s_bus_t bus);
int  drv_i2s_player_play_blocking(drv_i2s_player_t *p,
                                  const uint8_t *wav_data, uint32_t wav_len);

#endif