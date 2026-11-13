#ifndef __DRV_WAV_H
#define __DRV_WAV_H

#include "gd32f4xx.h"

typedef struct {
    uint16_t        format_tag;
    uint16_t        num_channels;
    uint32_t        sample_rate;
    uint16_t        bits_per_sample;
    uint32_t        data_size;
    uint32_t        total_frames;
    const uint8_t  *pcm_data;
} drv_wav_info_t;

typedef struct {
    const drv_wav_info_t *info;
    uint32_t frame_idx;
    int      channel;
    uint16_t mono_cache;
    int      mono_sent;
} drv_wav_iter_t;

int      drv_wav_parse(const uint8_t *data, uint32_t len, drv_wav_info_t *info);
void     drv_wav_iter_init(drv_wav_iter_t *it, const drv_wav_info_t *info);
uint16_t drv_wav_iter_next(drv_wav_iter_t *it);

#endif