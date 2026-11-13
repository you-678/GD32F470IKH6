#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"
#include "drv_i2s_player.h"

static drv_i2s_player_t s_player;

static void test_hw(void)
{
    printf("\r\n---- HARDWARE I2S ----\r\n");
    bsp_i2s_init_mode(BSP_I2S_BUS_AUDIO, BSP_I2S_MODE_HW);

    int rc = drv_i2s_player_play_blocking(&s_player, wavetestdata, wavetestdata_len);
    if (rc != 0) {
        printf("[HW] FAILED rc=%d\r\n", rc);
        return;
    }
    printf("[HW] rate=%lu ch=%u bits=%u frames=%lu\r\n",
           (unsigned long)s_player.wav.sample_rate,
           s_player.wav.num_channels,
           s_player.wav.bits_per_sample,
           (unsigned long)s_player.wav.total_frames);
    printf("[HW] PASS\r\n");
}

static void test_sw(void)
{
    printf("\r\n---- SOFTWARE I2S ----\r\n");
    bsp_i2s_init_mode(BSP_I2S_BUS_AUDIO, BSP_I2S_MODE_SW);

    int rc = drv_i2s_player_play_blocking(&s_player, wavetestdata, wavetestdata_len);
    if (rc != 0) {
        printf("[SW] FAILED rc=%d\r\n", rc);
        return;
    }
    printf("[SW] PASS\r\n");
}

int main(void)
{
    systick_config();
	bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
    nvic_irq_enable(BSP_I2S_AUDIO_IRQn, 0, 1);

    drv_i2s_player_init(&s_player, BSP_I2S_BUS_AUDIO);

    printf("\r\n===== I2S Player Self Test =====\r\n");
    printf("WAV: %lu bytes\r\n", (unsigned long)wavetestdata_len);

    test_hw();
    delay_1ms(500);
    test_sw();

    printf("\r\n===== ALL DONE =====\r\n");

    while (1) { delay_1ms(1000); }
}