#include "bsp_spi.h"

/* ============================================================
 * SPI_QCTL 寄存器位定义（GD32F4xx 用户手册）
 * 偏移 0x80，QMOD=bit0, QRD=bit1, IO23_DRV=bit2
 * ============================================================ */
#define SPI_QCTL_ADDR(spix)   ((spix) + 0x80U)
#define QCTL_QMOD             (1U << 0)   /* Quad 模式使能 */
#define QCTL_QRD              (1U << 1)   /* Quad 读选择：1=读，0=写 */
#define QCTL_IO23_DRV         (1U << 2)   /* 非 Quad 下驱动 IO2/IO3 */

/* ===== 静态硬件配置 ===== */
typedef struct {
    rcu_periph_enum  gpio_rcu;
    uint32_t         gpio_port;
    uint32_t         sck_pin;
    uint32_t         miso_pin;
    uint32_t         mosi_pin;
    uint32_t         io2_pin;
    uint32_t         io3_pin;
    uint32_t         gpio_af;

    uint32_t         spi;
    rcu_periph_enum  spi_rcu;
    uint32_t         prescale;

    rcu_periph_enum  cs_rcu;
    uint32_t         cs_port;
    uint32_t         cs_pin;
} bsp_spi_hw_t;

static const bsp_spi_hw_t s_hw[BSP_SPI_BUS_MAX] = {
    [BSP_SPI_BUS_FLASH] = {
        .gpio_rcu  = BSP_SPI_FLASH_GPIO_RCU,
        .gpio_port = BSP_SPI_FLASH_GPIO_PORT,
        .sck_pin   = BSP_SPI_FLASH_SCK_PIN,
        .miso_pin  = BSP_SPI_FLASH_MISO_PIN,
        .mosi_pin  = BSP_SPI_FLASH_MOSI_PIN,
        .io2_pin   = BSP_SPI_FLASH_IO2_PIN,
        .io3_pin   = BSP_SPI_FLASH_IO3_PIN,
        .gpio_af   = BSP_SPI_FLASH_GPIO_AF,

        .spi       = BSP_SPI_FLASH_PERIPH,
        .spi_rcu   = BSP_SPI_FLASH_RCU,
        .prescale  = BSP_SPI_FLASH_PRESCALE,

        .cs_rcu    = BSP_SPI_FLASH_CS_RCU,
        .cs_port   = BSP_SPI_FLASH_CS_PORT,
        .cs_pin    = BSP_SPI_FLASH_CS_PIN,
    },
};

/* ===== 运行时状态 ===== */
typedef struct {
    bsp_spi_mode_t mode;
    uint8_t        initialized;
} bsp_spi_rt_t;

static bsp_spi_rt_t s_rt[BSP_SPI_BUS_MAX] = {
    [BSP_SPI_BUS_FLASH] = {
        .mode        = (BSP_SPI_FLASH_USE_SW ? BSP_SPI_MODE_SW : BSP_SPI_MODE_HW),
        .initialized = 0,
    },
};

/* ===== 软件延时 ===== */
static inline void sw_spi_delay(void)
{
    for (volatile uint32_t i = 0; i < BSP_SPI_SW_DELAY_LOOPS; i++) {
        __NOP();
    }
}

/* ===== 引脚配置 ===== */
static void pins_to_af(const bsp_spi_hw_t *h)
{
    uint32_t pins = h->sck_pin | h->miso_pin | h->mosi_pin;
    if (h->io2_pin) pins |= h->io2_pin;
    if (h->io3_pin) pins |= h->io3_pin;

    gpio_af_set(h->gpio_port, h->gpio_af, pins);
    gpio_mode_set(h->gpio_port, GPIO_MODE_AF, GPIO_PUPD_NONE, pins);
    gpio_output_options_set(h->gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pins);
}

static void pins_to_gpio(const bsp_spi_hw_t *h)
{
    gpio_mode_set(h->gpio_port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                  h->sck_pin | h->mosi_pin);
    gpio_output_options_set(h->gpio_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                            h->sck_pin | h->mosi_pin);
    gpio_mode_set(h->gpio_port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, h->miso_pin);

    gpio_bit_reset(h->gpio_port, h->sck_pin);
    gpio_bit_reset(h->gpio_port, h->mosi_pin);
}

/* ===== 初始化 / 切换 ===== */
void bsp_spi_init_mode(bsp_spi_bus_t bus, bsp_spi_mode_t mode)
{
    const bsp_spi_hw_t *h = &s_hw[bus];
    bsp_spi_rt_t       *r = &s_rt[bus];

    gpio_bit_set(h->cs_port, h->cs_pin);

    if (r->initialized && r->mode == BSP_SPI_MODE_HW) {
        spi_disable(h->spi);
    }

    rcu_periph_clock_enable(h->gpio_rcu);
    rcu_periph_clock_enable(h->cs_rcu);
    if (mode == BSP_SPI_MODE_HW) {
        rcu_periph_clock_enable(h->spi_rcu);
    }

    if (mode == BSP_SPI_MODE_HW) {
        pins_to_af(h);

        spi_parameter_struct p;
        p.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
        p.device_mode          = SPI_MASTER;
        p.frame_size           = SPI_FRAMESIZE_8BIT;
        p.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        p.nss                  = SPI_NSS_SOFT;
        p.prescale             = h->prescale;
        p.endian               = SPI_ENDIAN_MSB;
        spi_init(h->spi, &p);
        spi_enable(h->spi);
    } else {
        pins_to_gpio(h);
    }

    gpio_mode_set(h->cs_port, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, h->cs_pin);
    gpio_output_options_set(h->cs_port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, h->cs_pin);
    gpio_bit_set(h->cs_port, h->cs_pin);

    r->mode        = mode;
    r->initialized = 1;
}

void bsp_spi_init(bsp_spi_bus_t bus)
{
    bsp_spi_init_mode(bus, s_rt[bus].mode);
}

bsp_spi_mode_t bsp_spi_get_mode(bsp_spi_bus_t bus)
{
    return s_rt[bus].mode;
}

/* ===== 收发 ===== */
static uint8_t sw_spi_xfer(const bsp_spi_hw_t *h, uint8_t tx)
{
    uint8_t rx = 0;
    for (int i = 7; i >= 0; i--) {
        if (tx & (1u << i)) gpio_bit_set  (h->gpio_port, h->mosi_pin);
        else                gpio_bit_reset(h->gpio_port, h->mosi_pin);
        sw_spi_delay();

        gpio_bit_set(h->gpio_port, h->sck_pin);
        sw_spi_delay();

        if (gpio_input_bit_get(h->gpio_port, h->miso_pin)) {
            rx |= (uint8_t)(1u << i);
        }

        gpio_bit_reset(h->gpio_port, h->sck_pin);
        sw_spi_delay();
    }
    return rx;
}

uint8_t bsp_spi_transfer(bsp_spi_bus_t bus, uint8_t tx)
{
    const bsp_spi_hw_t *h = &s_hw[bus];

    if (s_rt[bus].mode == BSP_SPI_MODE_SW) {
        return sw_spi_xfer(h, tx);
    }

    while (RESET == spi_i2s_flag_get(h->spi, SPI_FLAG_TBE)) { }
    spi_i2s_data_transmit(h->spi, tx);
    while (RESET == spi_i2s_flag_get(h->spi, SPI_FLAG_RBNE)) { }
    return (uint8_t)spi_i2s_data_receive(h->spi);
}

void bsp_spi_transfer_buf(bsp_spi_bus_t bus, const uint8_t *tx, uint8_t *rx, uint32_t len)
{
    while (len--) {
        uint8_t in = bsp_spi_transfer(bus, tx ? *tx++ : 0xFF);
        if (rx) *rx++ = in;
    }
}

/* ===== 片选 ===== */
void bsp_spi_cs_low(bsp_spi_bus_t bus)
{
    gpio_bit_reset(s_hw[bus].cs_port, s_hw[bus].cs_pin);
}

void bsp_spi_cs_high(bsp_spi_bus_t bus)
{
    gpio_bit_set(s_hw[bus].cs_port, s_hw[bus].cs_pin);
}

/* ============================================================
 * Quad 钩子（直接操作 SPI_QCTL 寄存器）
 * ============================================================ */

int bsp_spi_supports_quad(bsp_spi_bus_t bus)
{
    const bsp_spi_hw_t *h = &s_hw[bus];
    return (s_rt[bus].mode == BSP_SPI_MODE_HW &&
            h->io2_pin != 0 && h->io3_pin != 0) ? 1 : 0;
}

/*
 * 进入 Quad 读模式：QMOD=1, QRD=1
 * 时序要求：确认 TBE=1 且 TRANS=0 后再置位
 */
void bsp_spi_quad_enter_read(bsp_spi_bus_t bus)
{
    if (!bsp_spi_supports_quad(bus)) return;

    uint32_t spix = s_hw[bus].spi;

    /* 等待发送缓冲区空、传输完成 */
    while (RESET == spi_i2s_flag_get(spix, SPI_FLAG_TBE)) { }
    while (SET == spi_i2s_flag_get(spix, SPI_FLAG_TRANS)) { }

    /* QMOD=1, QRD=1 → Quad 读 */
    REG32(SPI_QCTL_ADDR(spix)) = QCTL_QMOD | QCTL_QRD;
}

/*
 * 进入 Quad 写模式：QMOD=1, QRD=0
 * 时序要求：确认 TBE=1 且 TRANS=0 后再置位
 */
void bsp_spi_quad_enter_write(bsp_spi_bus_t bus)
{
    if (!bsp_spi_supports_quad(bus)) return;

    uint32_t spix = s_hw[bus].spi;

    while (RESET == spi_i2s_flag_get(spix, SPI_FLAG_TBE)) { }
    while (SET == spi_i2s_flag_get(spix, SPI_FLAG_TRANS)) { }

    /* QMOD=1, QRD=0 → Quad 写 */
    REG32(SPI_QCTL_ADDR(spix)) = QCTL_QMOD;
}

/*
 * 退出 Quad 模式：清 QMOD，回到标准单线 SPI
 * 用户手册要求：先确认 TBE=1、TRANS=0，再清 QMOD
 */
void bsp_spi_quad_exit(bsp_spi_bus_t bus)
{
    if (s_rt[bus].mode != BSP_SPI_MODE_HW) return;

    uint32_t spix = s_hw[bus].spi;

    while (RESET == spi_i2s_flag_get(spix, SPI_FLAG_TBE)) { }
    while (SET == spi_i2s_flag_get(spix, SPI_FLAG_TRANS)) { }

    /* 清 QMOD */
    REG32(SPI_QCTL_ADDR(spix)) &= ~QCTL_QMOD;
}