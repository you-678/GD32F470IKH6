#include "drv_wav.h"

static uint16_t rd16(const uint8_t *p) { uint16_t v; memcpy(&v, p, 2); return v; }
static uint32_t rd32(const uint8_t *p) { uint32_t v; memcpy(&v, p, 4); return v; }

int drv_wav_parse(const uint8_t *data, uint32_t len, drv_wav_info_t *info)
{
    if (!data || !info || len < 44U) return -1;
    if (memcmp(data, "RIFF", 4) != 0) return -2;
    if (memcmp(data + 8, "WAVE", 4) != 0) return -3;

    memset(info, 0, sizeof(*info));
    uint32_t off = 12;
    int found_fmt = 0, found_data = 0;

    while (off + 8U <= len) {
        uint32_t sz = rd32(data + off + 4);
        if (memcmp(data + off, "fmt ", 4) == 0) {
            if (off + 24U > len) return -4;
            info->format_tag      = rd16(data + off + 8);
            info->num_channels    = rd16(data + off + 10);
            info->sample_rate     = rd32(data + off + 12);
            info->bits_per_sample = rd16(data + off + 22);
            found_fmt = 1;
        } else if (memcmp(data + off, "data", 4) == 0) {
            info->pcm_data  = data + off + 8;
            info->data_size = sz;
            if (off + 8U + sz > len) info->data_size = len - off - 8U;
            found_data = 1;
            break;
        }
        off += 8U + sz;
        if (sz & 1U) off++;
    }

    if (!found_fmt || !found_data) return -5;
    if (info->format_tag != 1U)       return -6;
    if (info->bits_per_sample != 16U) return -7;
    if (info->num_channels == 0U)     return -8;

    info->total_frames = info->data_size / (info->num_channels * 2U);
    return 0;
}

void drv_wav_iter_init(drv_wav_iter_t *it, const drv_wav_info_t *info)
{
    memset(it, 0, sizeof(*it));
    it->info = info;
}

uint16_t drv_wav_iter_next(drv_wav_iter_t *it)
{
    const drv_wav_info_t *w = it->info;
    if (w->total_frames == 0U) return 0;

    if (w->num_channels == 2U) {
        uint32_t off = it->frame_idx * 4U + (uint32_t)it->channel * 2U;
        uint16_t s = rd16(w->pcm_data + off);
        it->channel ^= 1;
        if (it->channel == 0) {
            it->frame_idx++;
            if (it->frame_idx >= w->total_frames) it->frame_idx = 0;
        }
        return s;
    } else {
        if (!it->mono_sent) {
            uint32_t off = it->frame_idx * 2U;
            it->mono_cache = rd16(w->pcm_data + off);
            it->mono_sent = 1;
            return it->mono_cache;
        } else {
            it->mono_sent = 0;
            it->frame_idx++;
            if (it->frame_idx >= w->total_frames) it->frame_idx = 0;
            return it->mono_cache;
        }
    }
}