#ifndef __DRV_SDCARD_H
#define __DRV_SDCARD_H

#include "gd32f4xx.h"

/* ===== 错误码 ===== */
typedef enum {
    SD_OK = 0,
    SD_ERROR,
    SD_CMD_CRC_ERROR, SD_CMD_RESP_TIMEOUT,
    SD_DATA_CRC_ERROR, SD_DATA_TIMEOUT,
    SD_TX_UNDERRUN_ERROR, SD_RX_OVERRUN_ERROR, SD_START_BIT_ERROR,
    SD_PARAMETER_INVALID, SD_FUNCTION_UNSUPPORTED, SD_LOCK_UNLOCK_FAILED,
    SD_VOLTRANGE_INVALID, SD_OPERATION_IMPROPER,
    SD_OUT_OF_RANGE, SD_ADDRESS_ERROR, SD_BLOCK_LEN_ERROR,
    SD_ERASE_SEQ_ERROR, SD_ERASE_PARAM, SD_WP_VIOLATION,
    SD_COM_CRC_ERROR, SD_ILLEGAL_COMMAND, SD_CARD_ECC_FAILED,
    SD_CC_ERROR, SD_GENERAL_UNKNOWN_ERROR, SD_CSD_OVERWRITE,
    SD_WP_ERASE_SKIP, SD_CARD_ECC_DISABLED, SD_ERASE_RESET, SD_AKE_SEQ_ERROR,
} sd_error_typedef_enum;

typedef enum {
    SD_NO_TRANSFER = 0,
    SD_TRANSFER_IN_PROGRESS
} sd_transfer_state_typedef_enum;

/* ===== 锁/解锁 ===== */
#define SD_LOCK     0x05U
#define SD_UNLOCK   0x00U

/* ===== 传输模式 ===== */
#define SD_DMA_MODE        0x01U
#define SD_POLLING_MODE    0x00U

/* ===== CCC 位 ===== */
#define SD_CCC_BLOCK_READ           ((uint16_t)0x0004)
#define SD_CCC_BLOCK_WRITE          ((uint16_t)0x0010)
#define SD_CCC_ERASE                ((uint16_t)0x0020)
#define SD_CCC_WRITE_PROTECTION     ((uint16_t)0x0040)
#define SD_CCC_LOCK_CARD            ((uint16_t)0x0080)
#define SD_CCC_APPLICATION_SPECIFIC ((uint16_t)0x0100)
#define SD_CCC_IO_MODE              ((uint16_t)0x0200)
#define SD_CCC_SWITCH               ((uint16_t)0x0400)

/* ===== CID / CSD / 卡信息 ===== */
typedef struct {
    uint8_t  mid;
    uint16_t oid;
    uint32_t pnm0;
    uint8_t  pnm1;
    uint8_t  prv;
    uint32_t psn;
    uint16_t mdt;
    uint8_t  cid_crc;
} sd_cid_struct;

typedef struct {
    uint8_t  csd_struct;
    uint8_t  taac;
    uint8_t  nsac;
    uint8_t  tran_speed;
    uint16_t ccc;
    uint8_t  read_bl_len;
    uint8_t  read_bl_partial;
    uint8_t  write_blk_misalign;
    uint8_t  read_blk_misalign;
    uint8_t  dsp_imp;
    uint32_t c_size;
    uint8_t  vdd_r_curr_min;
    uint8_t  vdd_r_curr_max;
    uint8_t  vdd_w_curr_min;
    uint8_t  vdd_w_curr_max;
    uint8_t  c_size_mult;
    uint8_t  erase_blk_en;
    uint8_t  sector_size;
    uint8_t  wp_grp_size;
    uint8_t  wp_grp_enable;
    uint8_t  r2w_factor;
    uint8_t  write_bl_len;
    uint8_t  write_bl_partial;
    uint8_t  file_format_grp;
    uint8_t  copy_flag;
    uint8_t  perm_write_protect;
    uint8_t  tmp_write_protect;
    uint8_t  file_format;
    uint8_t  csd_crc;
} sd_csd_struct;

typedef struct {
    uint32_t 			card_type;
    uint16_t            card_rca;
    sd_cid_struct       card_cid;
    sd_csd_struct       card_csd;
    uint32_t            card_blocksize;
    uint32_t            card_capacity;
} sd_card_info_struct;

extern uint32_t sd_scr[2];

/* ===== 对外接口 ===== */
sd_error_typedef_enum sd_init(void);
sd_error_typedef_enum sd_card_init(void);
sd_error_typedef_enum sd_power_on(void);
sd_error_typedef_enum sd_power_off(void);
sd_error_typedef_enum sd_bus_mode_config(uint32_t busmode);
sd_error_typedef_enum sd_transfer_mode_config(uint32_t txmode);

sd_error_typedef_enum sd_block_read(uint32_t *preadbuffer, uint32_t readaddr, uint16_t blocksize);
sd_error_typedef_enum sd_multiblocks_read(uint32_t *preadbuffer, uint32_t readaddr, uint16_t blocksize, uint32_t blocksnumber);
sd_error_typedef_enum sd_block_write(uint32_t *pwritebuffer, uint32_t writeaddr, uint16_t blocksize);
sd_error_typedef_enum sd_multiblocks_write(uint32_t *pwritebuffer, uint32_t writeaddr, uint16_t blocksize, uint32_t blocksnumber);

sd_error_typedef_enum sd_erase(uint32_t startaddr, uint32_t endaddr);
sd_error_typedef_enum sd_interrupts_process(void);
sd_error_typedef_enum sd_card_select_deselect(uint16_t cardrca);
sd_error_typedef_enum sd_cardstatus_get(uint32_t *pcardstatus);
sd_error_typedef_enum sd_sdstatus_get(uint32_t *psdstatus);
sd_error_typedef_enum sd_transfer_stop(void);
sd_error_typedef_enum sd_lock_unlock(uint8_t lockstate);
sd_transfer_state_typedef_enum sd_transfer_state_get(void);
uint32_t sd_card_capacity_get(void);
sd_error_typedef_enum sd_card_information_get(sd_card_info_struct *pcardinfo);

#endif