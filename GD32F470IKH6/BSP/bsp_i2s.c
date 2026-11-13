#include "bsp_i2s.h"

typedef struct {
    bsp_i2s_mode_t  mode;
    uint8_t         initialized;
    bsp_i2s_tx_cb_t cb;
    void           *ctx;
    volatile uint32_t remaining;
    volatile uint8_t  done;
} bsp_i2s_rt_t;

static bsp_i2s_rt_t s_rt[BSP_I2S_BUS_MAX];

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static void hw_pins_af(void)
{
    gpio_af_set(BSP_I2S_MCK_PORT, BSP_I2S_MCK_AF, BSP_I2S_MCK_PIN);
    gpio_mode_set(BSP_I2S_MCK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_I2S_MCK_PIN);
    gpio_output_options_set(BSP_I2S_MCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_I2S_MCK_PIN);

    gpio_af_set(BSP_I2S_CKWS_PORT, BSP_I2S_CKWS_AF, BSP_I2S_CKWS_PINS);
    gpio_mode_set(BSP_I2S_CKWS_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_I2S_CKWS_PINS);
    gpio_output_options_set(BSP_I2S_CKWS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_I2S_CKWS_PINS);

    gpio_af_set(BSP_I2S_SD_PORT, BSP_I2S_SD_AF, BSP_I2S_SD_PIN);
    gpio_mode_set(BSP_I2S_SD_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_I2S_SD_PIN);
    gpio_output_options_set(BSP_I2S_SD_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_I2S_SD_PIN);
}

static void sw_pins_gpio(void)
{
    gpio_mode_set(BSP_I2S_MCK_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, BSP_I2S_MCK_PIN);

    gpio_mode_set(BSP_I2S_CKWS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_I2S_CKWS_PINS);
    gpio_output_options_set(BSP_I2S_CKWS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_I2S_CKWS_PINS);
    gpio_bit_reset(BSP_I2S_CKWS_PORT, BSP_I2S_CK_PIN);
    gpio_bit_reset(BSP_I2S_CKWS_PORT, BSP_I2S_WS_PIN);

    gpio_mode_set(BSP_I2S_SD_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_I2S_SD_PIN);
    gpio_output_options_set(BSP_I2S_SD_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_I2S_SD_PIN);
    gpio_bit_reset(BSP_I2S_SD_PORT, BSP_I2S_SD_PIN);
}

void bsp_i2s_init_mode(bsp_i2s_bus_t bus, bsp_i2s_mode_t mode)
{
    bsp_i2s_rt_t *r = &s_rt[bus];

    if (r->initialized && r->mode == BSP_I2S_MODE_HW) {
        spi_i2s_interrupt_disable(BSP_I2S_AUDIO_PERIPH, SPI_I2S_INT_TBE);
        i2s_disable(BSP_I2S_AUDIO_PERIPH);
    }

    rcu_periph_clock_enable(BSP_I2S_MCK_RCU);
    rcu_periph_clock_enable(BSP_I2S_CKWS_RCU);
    rcu_periph_clock_enable(BSP_I2S_SD_RCU);

    if (mode == BSP_I2S_MODE_HW) {
        rcu_periph_clock_enable(BSP_I2S_AUDIO_RCU);
        hw_pins_af();
    } else {
        dwt_init();
        sw_pins_gpio();
    }

    memset(r, 0, sizeof(*r));
    r->mode = mode;
    r->initialized = 1;
    r->done = 1;
}

bsp_i2s_mode_t bsp_i2s_get_mode(bsp_i2s_bus_t bus)
{
    return s_rt[bus].mode;
}

static void hw_configure(uint32_t sample_rate)
{
    spi_i2s_deinit(BSP_I2S_AUDIO_PERIPH);
    i2s_psc_config(BSP_I2S_AUDIO_PERIPH, sample_rate,
                   I2S_FRAMEFORMAT_DT16B_CH16B, I2S_MCKOUT_ENABLE);
    i2s_init(BSP_I2S_AUDIO_PERIPH, I2S_MODE_MASTERTX, I2S_STD_MSB, I2S_CKPL_HIGH);
    i2s_enable(BSP_I2S_AUDIO_PERIPH);
}

static inline void sw_delay(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles) { }
}

static void sw_write_frame(uint16_t left, uint16_t right, uint32_t half_bit)
{
    gpio_bit_reset(BSP_I2S_CKWS_PORT, BSP_I2S_WS_PIN);
    for (int i = 15; i >= 0; i--) {
        gpio_bit_reset(BSP_I2S_CKWS_PORT, BSP_I2S_CK_PIN);
        if ((left >> i) & 1U) gpio_bit_set(BSP_I2S_SD_PORT, BSP_I2S_SD_PIN);
        else                  gpio_bit_reset(BSP_I2S_SD_PORT, BSP_I2S_SD_PIN);
        sw_delay(half_bit);
        gpio_bit_set(BSP_I2S_CKWS_PORT, BSP_I2S_CK_PIN);
        sw_delay(half_bit);
    }

    gpio_bit_set(BSP_I2S_CKWS_PORT, BSP_I2S_WS_PIN);
    for (int i = 15; i >= 0; i--) {
        gpio_bit_reset(BSP_I2S_CKWS_PORT, BSP_I2S_CK_PIN);
        if ((right >> i) & 1U) gpio_bit_set(BSP_I2S_SD_PORT, BSP_I2S_SD_PIN);
        else                   gpio_bit_reset(BSP_I2S_SD_PORT, BSP_I2S_SD_PIN);
        sw_delay(half_bit);
        gpio_bit_set(BSP_I2S_CKWS_PORT, BSP_I2S_CK_PIN);
        sw_delay(half_bit);
    }
}

void bsp_i2s_start(bsp_i2s_bus_t bus, uint32_t sample_rate_hz,
                   bsp_i2s_tx_cb_t cb, void *ctx, uint32_t num_halfwords)
{
    bsp_i2s_rt_t *r = &s_rt[bus];

    r->cb = cb;
    r->ctx = ctx;
    r->remaining = num_halfwords;
    r->done = 0;

    if (r->mode == BSP_I2S_MODE_HW) {
        hw_configure(sample_rate_hz);
        spi_i2s_interrupt_enable(BSP_I2S_AUDIO_PERIPH, SPI_I2S_INT_TBE);
    } else {
        uint32_t cyc = SystemCoreClock / (sample_rate_hz * 32U);
        uint32_t half = cyc / 2U;
        if (half < 2U) half = 2U;

        uint32_t frames = num_halfwords / 2U;
        for (uint32_t f = 0; f < frames; f++) {
            uint16_t l = cb ? cb(ctx) : 0;
            uint16_t rr = cb ? cb(ctx) : 0;
            sw_write_frame(l, rr, half);
        }
        r->done = 1;
    }
}

void bsp_i2s_stop(bsp_i2s_bus_t bus)
{
    bsp_i2s_rt_t *r = &s_rt[bus];
    if (r->mode == BSP_I2S_MODE_HW) {
        spi_i2s_interrupt_disable(BSP_I2S_AUDIO_PERIPH, SPI_I2S_INT_TBE);
        while (RESET == spi_i2s_flag_get(BSP_I2S_AUDIO_PERIPH, SPI_FLAG_TBE)) { }
        i2s_disable(BSP_I2S_AUDIO_PERIPH);
    }
    r->done = 1;
}

int bsp_i2s_is_done(bsp_i2s_bus_t bus)
{
    return s_rt[bus].done;
}

void bsp_i2s_irq_handler(bsp_i2s_bus_t bus)
{
    bsp_i2s_rt_t *r = &s_rt[bus];
    if (r->mode != BSP_I2S_MODE_HW) return;

    if (spi_i2s_interrupt_flag_get(BSP_I2S_AUDIO_PERIPH, SPI_I2S_INT_TBE)) {
        if (r->remaining > 0U) {
            uint16_t s = r->cb ? r->cb(r->ctx) : 0;
            spi_i2s_data_transmit(BSP_I2S_AUDIO_PERIPH, s);
            r->remaining--;
        } else {
            spi_i2s_interrupt_disable(BSP_I2S_AUDIO_PERIPH, SPI_I2S_INT_TBE);
            r->done = 1;
        }
    }
}