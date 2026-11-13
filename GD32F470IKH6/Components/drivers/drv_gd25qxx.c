#include "drv_gd25qxx.h"

/* ===== SPI 指令 ===== */
#define CMD_WRITE_ENABLE        0x06
#define CMD_READ_STATUS1        0x05
#define CMD_READ_STATUS2        0x35
#define CMD_WRITE_STATUS        0x01
#define CMD_READ_DATA           0x03
#define CMD_QUAD_READ           0x6B
#define CMD_PAGE_PROGRAM        0x02
#define CMD_QUAD_PAGE_PROGRAM   0x32
#define CMD_SECTOR_ERASE        0x20
#define CMD_BLOCK32_ERASE       0x52
#define CMD_BLOCK64_ERASE       0xD8
#define CMD_CHIP_ERASE          0xC7
#define CMD_READ_ID             0x9F
#define CMD_DUMMY               0xFF

#define QUAD_DUMMY_BYTES        4
#define WRITE_TIMEOUT_LOOPS     0x00FFFFFFu

/* ===== 内部辅助 ===== */
static void send_cmd(gd25qxx_dev_t *d, uint8_t c) { d->spi_xfer(c); }

static void send_addr24(gd25qxx_dev_t *d, uint32_t a)
{
    d->spi_xfer((uint8_t)(a >> 16));
    d->spi_xfer((uint8_t)(a >> 8));
    d->spi_xfer((uint8_t)(a));
}

static uint32_t id_to_capacity(uint32_t id)
{
    switch (id & 0xFFu) {
    case 0x13: return 1u << 19;   /* 512 KB  */
    case 0x14: return 1u << 20;   /* 1 MB    */
    case 0x15: return 1u << 21;   /* 2 MB    */
    case 0x16: return 1u << 22;   /* 4 MB    */
    case 0x17: return 1u << 23;   /* 8 MB    */
    case 0x18: return 1u << 24;   /* 16 MB   */
    case 0x19: return 1u << 25;   /* 32 MB（需 4B 地址） */
    default:   return 0;
    }
}

static inline int quad_available(gd25qxx_dev_t *d)
{
    return (d->quad_enter_read  != NULL) &&
           (d->quad_enter_write != NULL) &&
           (d->quad_exit        != NULL);
}

/* ===== 绑定 ===== */
void gd25qxx_bind(gd25qxx_dev_t *d,
                  uint8_t (*spi_xfer)(uint8_t),
                  void (*cs_low)(void),
                  void (*cs_high)(void),
                  void (*quad_enter_read)(void),
                  void (*quad_enter_write)(void),
                  void (*quad_exit)(void))
{
    memset(d, 0, sizeof(*d));
    d->spi_xfer         = spi_xfer;
    d->cs_low           = cs_low;
    d->cs_high          = cs_high;
    d->quad_enter_read  = quad_enter_read;
    d->quad_enter_write = quad_enter_write;
    d->quad_exit        = quad_exit;
}

/* ===== 基础操作 ===== */
uint32_t gd25qxx_read_id(gd25qxx_dev_t *d)
{
    uint32_t id;
    d->cs_low();
    send_cmd(d, CMD_READ_ID);
    id  = (uint32_t)d->spi_xfer(CMD_DUMMY) << 16;
    id |= (uint32_t)d->spi_xfer(CMD_DUMMY) << 8;
    id |= (uint32_t)d->spi_xfer(CMD_DUMMY);
    d->cs_high();
    return id;
}

int gd25qxx_probe(gd25qxx_dev_t *d)
{
    d->id       = gd25qxx_read_id(d);
    d->capacity = id_to_capacity(d->id);
    return (d->capacity != 0) ? GD25QXX_OK : GD25QXX_ERR_ID;
}

uint8_t gd25qxx_read_status1(gd25qxx_dev_t *d)
{
    uint8_t s;
    d->cs_low();
    send_cmd(d, CMD_READ_STATUS1);
    s = d->spi_xfer(CMD_DUMMY);
    d->cs_high();
    return s;
}

uint8_t gd25qxx_read_status2(gd25qxx_dev_t *d)
{
    uint8_t s;
    d->cs_low();
    send_cmd(d, CMD_READ_STATUS2);
    s = d->spi_xfer(CMD_DUMMY);
    d->cs_high();
    return s;
}

void gd25qxx_write_status(gd25qxx_dev_t *d, uint8_t sr1, uint8_t sr2)
{
    gd25qxx_write_enable(d);
    d->cs_low();
    send_cmd(d, CMD_WRITE_STATUS);
    d->spi_xfer(sr1);
    d->spi_xfer(sr2);
    d->cs_high();
    gd25qxx_wait_busy(d);
}

void gd25qxx_write_enable(gd25qxx_dev_t *d)
{
    d->cs_low();
    send_cmd(d, CMD_WRITE_ENABLE);
    d->cs_high();
}

int gd25qxx_wait_busy(gd25qxx_dev_t *d)
{
    uint32_t to = WRITE_TIMEOUT_LOOPS;
    while (gd25qxx_read_status1(d) & GD25QXX_SR1_WIP) {
        if (--to == 0) return GD25QXX_ERR_TIMEOUT;
    }
    return GD25QXX_OK;
}

/* ===== 擦除 ===== */
static int erase_op(gd25qxx_dev_t *d, uint8_t cmd, uint32_t addr, uint32_t align)
{
    if (d == NULL || (addr & (align - 1u)) != 0) return GD25QXX_ERR_PARAM;
    if (addr >= d->capacity)                     return GD25QXX_ERR_PARAM;

    gd25qxx_write_enable(d);
    d->cs_low();
    send_cmd(d, cmd);
    send_addr24(d, addr);
    d->cs_high();
    return gd25qxx_wait_busy(d);
}

int gd25qxx_erase_sector (gd25qxx_dev_t *d, uint32_t a) { return erase_op(d, CMD_SECTOR_ERASE,  a, GD25QXX_SECTOR_SIZE); }
int gd25qxx_erase_block32(gd25qxx_dev_t *d, uint32_t a) { return erase_op(d, CMD_BLOCK32_ERASE, a, GD25QXX_BLOCK32_SIZE); }
int gd25qxx_erase_block64(gd25qxx_dev_t *d, uint32_t a) { return erase_op(d, CMD_BLOCK64_ERASE, a, GD25QXX_BLOCK64_SIZE); }

int gd25qxx_erase_chip(gd25qxx_dev_t *d)
{
    gd25qxx_write_enable(d);
    d->cs_low();
    send_cmd(d, CMD_CHIP_ERASE);
    d->cs_high();
    return gd25qxx_wait_busy(d);
}

/* ===== 单线读写 ===== */
int gd25qxx_read(gd25qxx_dev_t *d, uint8_t *buf, uint32_t addr, uint32_t len)
{
    if (d == NULL || buf == NULL || len == 0) return GD25QXX_ERR_PARAM;
    if (addr + len > d->capacity)             return GD25QXX_ERR_PARAM;

    d->cs_low();
    send_cmd(d, CMD_READ_DATA);
    send_addr24(d, addr);
    while (len--) *buf++ = d->spi_xfer(CMD_DUMMY);
    d->cs_high();
    return GD25QXX_OK;
}

int gd25qxx_write(gd25qxx_dev_t *d, const uint8_t *buf, uint32_t addr, uint32_t len)
{
    if (d == NULL || buf == NULL || len == 0) return GD25QXX_ERR_PARAM;
    if (addr + len > d->capacity)             return GD25QXX_ERR_PARAM;

    while (len) {
        uint32_t page_off = addr % GD25QXX_PAGE_SIZE;
        uint32_t chunk    = GD25QXX_PAGE_SIZE - page_off;
        if (chunk > len) chunk = len;

        gd25qxx_write_enable(d);
        d->cs_low();
        send_cmd(d, CMD_PAGE_PROGRAM);
        send_addr24(d, addr);
        for (uint32_t i = 0; i < chunk; i++) d->spi_xfer(buf[i]);
        d->cs_high();

        if (gd25qxx_wait_busy(d) != GD25QXX_OK) return GD25QXX_ERR_TIMEOUT;

        addr += chunk; buf += chunk; len -= chunk;
    }
    return GD25QXX_OK;
}

/* ===== Quad 模式 ===== */
void gd25qxx_quad_enable(gd25qxx_dev_t *d)
{
    if (d == NULL || !quad_available(d)) return;
    uint8_t sr2 = gd25qxx_read_status2(d);
    if ((sr2 & GD25QXX_SR2_QE) == 0) {
        uint8_t sr1 = gd25qxx_read_status1(d);
        gd25qxx_write_status(d, sr1, (uint8_t)(sr2 | GD25QXX_SR2_QE));
    }
    d->quad_enabled = 1;
}

int gd25qxx_quad_read(gd25qxx_dev_t *d, uint8_t *buf, uint32_t addr, uint32_t len)
{
    if (d == NULL || buf == NULL || len == 0) return GD25QXX_ERR_PARAM;
    if (addr + len > d->capacity)             return GD25QXX_ERR_PARAM;
    if (!quad_available(d))                   return gd25qxx_read(d, buf, addr, len);

    d->cs_low();
    send_cmd(d, CMD_QUAD_READ);
    send_addr24(d, addr);

    d->quad_enter_read();
    for (int i = 0; i < QUAD_DUMMY_BYTES; i++) d->spi_xfer(CMD_DUMMY);
    while (len--) *buf++ = d->spi_xfer(CMD_DUMMY);

    d->cs_high();
    d->quad_exit();
    return GD25QXX_OK;
}

int gd25qxx_quad_write(gd25qxx_dev_t *d, const uint8_t *buf, uint32_t addr, uint32_t len)
{
    if (d == NULL || buf == NULL || len == 0) return GD25QXX_ERR_PARAM;
    if (addr + len > d->capacity)             return GD25QXX_ERR_PARAM;
    if (!quad_available(d))                   return gd25qxx_write(d, buf, addr, len);

    while (len) {
        uint32_t page_off = addr % GD25QXX_PAGE_SIZE;
        uint32_t chunk    = GD25QXX_PAGE_SIZE - page_off;
        if (chunk > len) chunk = len;

        gd25qxx_write_enable(d);
        d->cs_low();
        send_cmd(d, CMD_QUAD_PAGE_PROGRAM);
        send_addr24(d, addr);

        d->quad_enter_write();
        for (uint32_t i = 0; i < chunk; i++) d->spi_xfer(buf[i]);
        d->cs_high();
        d->quad_exit();

        if (gd25qxx_wait_busy(d) != GD25QXX_OK) return GD25QXX_ERR_TIMEOUT;

        addr += chunk; buf += chunk; len -= chunk;
    }
    return GD25QXX_OK;
}