#include "drv_i2s_player.h"

static uint16_t player_cb(void *ctx)
{
    drv_i2s_player_t *p = (drv_i2s_player_t*)ctx;
    return drv_wav_iter_next(&p->iter);
}

void drv_i2s_player_init(drv_i2s_player_t *p, bsp_i2s_bus_t bus)
{
    memset(p, 0, sizeof(*p));
    p->bus = bus;
}

int drv_i2s_player_play_blocking(drv_i2s_player_t *p,
                                 const uint8_t *wav_data, uint32_t wav_len)
{
    int rc = drv_wav_parse(wav_data, wav_len, &p->wav);
    if (rc != 0) return rc;

    drv_wav_iter_init(&p->iter, &p->wav);

    uint32_t n = p->wav.total_frames * 2U;

    bsp_i2s_start(p->bus, p->wav.sample_rate, player_cb, p, n);
    while (!bsp_i2s_is_done(p->bus)) { }
    bsp_i2s_stop(p->bus);
    return 0;
}