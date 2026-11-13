#ifndef __DRV_STORAGE_H
#define __DRV_STORAGE_H

#include "gd32f4xx.h"

typedef enum {
    STORAGE_MODE_SDRAM = 0,
    STORAGE_MODE_SDRAM_SLEEP,
    STORAGE_MODE_NAND,
    STORAGE_MODE_MAX
} storage_mode_t;

const char *drv_storage_mode_name(storage_mode_t mode);
int         drv_storage_run(storage_mode_t mode);

#endif