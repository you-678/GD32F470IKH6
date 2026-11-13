#ifndef __DRV_GD25QXX_H
#define __DRV_GD25QXX_H

#include "gd32f4xx.h"

/* ===== 容量常量 ===== */
#define GD25QXX_PAGE_SIZE      0x100u      /* 256 B  */
#define GD25QXX_SECTOR_SIZE    0x1000u     /* 4 KB   */
#define GD25QXX_BLOCK32_SIZE   0x8000u     /* 32 KB  */
#define GD25QXX_BLOCK64_SIZE   0x10000u    /* 64 KB  */

/* ===== 状态寄存器位 ===== */
#define GD25QXX_SR1_WIP        0x01        /* 写进行中 */
#define GD25QXX_SR1_WEL        0x02        /* 写使能锁存 */
#define GD25QXX_SR2_QE         0x02        /* Quad 使能（在 SR2 中） */

/* ===== 返回码 ===== */
#define GD25QXX_OK             0
#define GD25QXX_ERR_PARAM     -1           /* NULL / 越界 / 未对齐 */
#define GD25QXX_ERR_ID        -2           /* 读 ID 失败或容量未知 */
#define GD25QXX_ERR_TIMEOUT   -3           /* 等待 WIP 超时 */

/* ===== 设备句柄 ===== */
typedef struct gd25qxx_dev {
    /* 底层回调（由 BSP 层提供） */
    uint8_t (*spi_xfer)(uint8_t tx);
    void    (*cs_low)(void);
    void    (*cs_high)(void);

    /* Quad 回调（可选，为 NULL 时自动降级为单线） */
    void    (*quad_enter_read)(void);
    void    (*quad_enter_write)(void);
    void    (*quad_exit)(void);

    /* 运行时状态（驱动维护，调用方只读） */
    uint32_t id;
    uint32_t capacity;
    uint8_t  quad_enabled;
} gd25qxx_dev_t;

/* ===== 绑定 ===== */
void gd25qxx_bind(gd25qxx_dev_t *dev,
                  uint8_t (*spi_xfer)(uint8_t),
                  void (*cs_low)(void),
                  void (*cs_high)(void),
                  void (*quad_enter_read)(void),
                  void (*quad_enter_write)(void),
                  void (*quad_exit)(void));

/* ===== 探测 ===== */
int      gd25qxx_probe(gd25qxx_dev_t *dev);

/* ===== 基础操作 ===== */
uint32_t gd25qxx_read_id(gd25qxx_dev_t *dev);
uint8_t  gd25qxx_read_status1(gd25qxx_dev_t *dev);
uint8_t  gd25qxx_read_status2(gd25qxx_dev_t *dev);
void     gd25qxx_write_status(gd25qxx_dev_t *dev, uint8_t sr1, uint8_t sr2);
void     gd25qxx_write_enable(gd25qxx_dev_t *dev);
int      gd25qxx_wait_busy(gd25qxx_dev_t *dev);

/* ===== 擦除 ===== */
int gd25qxx_erase_sector (gd25qxx_dev_t *dev, uint32_t addr);   /* 4 KB  */
int gd25qxx_erase_block32(gd25qxx_dev_t *dev, uint32_t addr);   /* 32 KB */
int gd25qxx_erase_block64(gd25qxx_dev_t *dev, uint32_t addr);   /* 64 KB */
int gd25qxx_erase_chip   (gd25qxx_dev_t *dev);

/* ===== 单线读写 ===== */
int gd25qxx_read (gd25qxx_dev_t *dev, uint8_t *buf,       uint32_t addr, uint32_t len);
int gd25qxx_write(gd25qxx_dev_t *dev, const uint8_t *buf, uint32_t addr, uint32_t len);

/* ===== Quad 模式（无 quad 回调时自动走单线） ===== */
void gd25qxx_quad_enable(gd25qxx_dev_t *dev);
int  gd25qxx_quad_read (gd25qxx_dev_t *dev, uint8_t *buf,       uint32_t addr, uint32_t len);
int  gd25qxx_quad_write(gd25qxx_dev_t *dev, const uint8_t *buf, uint32_t addr, uint32_t len);

#endif /* __DRV_GD25QXX_H */