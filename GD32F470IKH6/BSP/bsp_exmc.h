#ifndef __BSP_EXMC_H
#define __BSP_EXMC_H

#include "gd32f4xx.h"

typedef enum {
    BSP_EXMC_MODE_SDRAM = 0,
    BSP_EXMC_MODE_NAND,
    BSP_EXMC_MODE_MAX
} bsp_exmc_mode_t;

typedef struct {
    uint8_t maker_id;
    uint8_t device_id;
    uint8_t third_id;
    uint8_t fourth_id;
} bsp_nand_id_t;

void            bsp_exmc_init_mode(bsp_exmc_mode_t mode);
bsp_exmc_mode_t bsp_exmc_get_mode(void);

void bsp_sdram_write(uint8_t *buf, uint32_t addr, uint32_t len);
void bsp_sdram_read (uint8_t *buf, uint32_t addr, uint32_t len);
void bsp_sdram_enter_self_refresh(void);
void bsp_sdram_exit_self_refresh(void);

void bsp_nand_read_id(bsp_nand_id_t *id);
int  bsp_nand_write(uint32_t memaddr, uint8_t *buf, uint16_t len);
int  bsp_nand_read (uint32_t memaddr, uint8_t *buf, uint16_t len);
int  bsp_nand_erase_block(uint32_t block);
int  bsp_nand_reset(void);

void bsp_exmc_restore_clock(void);

#endif /* __BSP_EXMC_H */