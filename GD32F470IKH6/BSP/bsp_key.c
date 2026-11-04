#include "bsp_key.h"

void bsp_key_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_mode_set(KEY_TAMPER_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, KEY_TAMPER_PIN);
}

/* 返回 0：按下；1：松开（和例程一致） */
uint8_t bsp_key_tamper_read(void)
{
    return gpio_input_bit_get(KEY_TAMPER_PORT, KEY_TAMPER_PIN);
}
