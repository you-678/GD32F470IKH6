#ifndef __BSP_SDIO_H
#define __BSP_SDIO_H

#include "gd32f4xx.h"

/* ===== SD 卡总线/传输模式配置（调试期先切 1BIT+POLLING） ===== */
//#define BUSMODE_4BIT
#define BUSMODE_1BIT

#define POLLING_MODE
//#define DMA_MODE

/* ===== 响应类型枚举 ===== */
typedef enum {
    BSP_SDIO_RESP_NONE = 0,
    BSP_SDIO_RESP_SHORT,
    BSP_SDIO_RESP_LONG
} bsp_sdio_resp_typedef_enum;

/* ===== 控制器初始化 ===== */
void bsp_sdio_init(void);
void bsp_sdio_clock_set(uint16_t clkdiv, uint32_t busmode);

/* ===== 命令与响应 ===== */
void     bsp_sdio_send_cmd(uint8_t cmd_index, uint32_t arg, bsp_sdio_resp_typedef_enum resp);
uint32_t bsp_sdio_get_response(uint8_t index);       /* index: 0~3 */
uint8_t  bsp_sdio_get_resp_cmd_index(void);

/* ===== FIFO 读写（轮询模式用） ===== */
uint32_t bsp_sdio_read_word(void);
void     bsp_sdio_write_word(uint32_t word);

/* ===== DMA 配置（DMA 模式用） ===== */
void    bsp_sdio_dma_tx_config(uint32_t *src, uint32_t bytes);
void    bsp_sdio_dma_rx_config(uint32_t *dst, uint32_t bytes);
uint8_t bsp_sdio_dma_is_done(void);

#endif