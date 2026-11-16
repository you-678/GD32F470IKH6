/*!
    \file    drv_sdcard.c
    \brief   SD card driver（分层版：依赖 bsp_sdio，不直接操作 GPIO/DMA 寄存器）
*/

#include "drv_sdcard.h"

/* ================================================================
 *               手动补全：SD 命令号 / 应用命令号
 *   这些定义原厂放在 sdcard.h 里，不在 gd32f4xx_sdio.h 中
 * ================================================================ */
#ifndef SD_CMD_GO_IDLE_STATE
#define SD_CMD_GO_IDLE_STATE                ((uint8_t)0U)
#define SD_CMD_ALL_SEND_CID                 ((uint8_t)2U)
#define SD_CMD_SEND_RELATIVE_ADDR           ((uint8_t)3U)
#define SD_CMD_SELECT_DESELECT_CARD         ((uint8_t)7U)
#define SD_CMD_SEND_IF_COND                 ((uint8_t)8U)
#define SD_CMD_SEND_CSD                     ((uint8_t)9U)
#define SD_CMD_SEND_CID                     ((uint8_t)10U)
#define SD_CMD_STOP_TRANSMISSION            ((uint8_t)12U)
#define SD_CMD_SEND_STATUS                  ((uint8_t)13U)
#define SD_CMD_SET_BLOCKLEN                 ((uint8_t)16U)
#define SD_CMD_READ_SINGLE_BLOCK            ((uint8_t)17U)
#define SD_CMD_READ_MULTIPLE_BLOCK          ((uint8_t)18U)
#define SD_CMD_WRITE_BLOCK                  ((uint8_t)24U)
#define SD_CMD_WRITE_MULTIPLE_BLOCK         ((uint8_t)25U)
#define SD_CMD_ERASE_WR_BLK_START           ((uint8_t)32U)
#define SD_CMD_ERASE_WR_BLK_END             ((uint8_t)33U)
#define SD_CMD_ERASE                        ((uint8_t)38U)
#define SD_CMD_LOCK_UNLOCK                  ((uint8_t)42U)
#define SD_CMD_APP_CMD                      ((uint8_t)55U)
#endif

#ifndef SD_APPCMD_SET_BUS_WIDTH
#define SD_APPCMD_SET_BUS_WIDTH             ((uint8_t)6U)
#define SD_APPCMD_SD_STATUS                 ((uint8_t)13U)
#define SD_APPCMD_SET_WR_BLK_ERASE_COUNT    ((uint8_t)23U)
#define SD_APPCMD_SD_SEND_OP_COND           ((uint8_t)41U)
#define SD_APPCMD_SEND_SCR                  ((uint8_t)51U)
#endif

/* ================================================================
 *               手动补全：卡类型（若库头已有则跳过）
 * ================================================================ */
#ifndef SDIO_STD_CAPACITY_SD_CARD_V1_1
#define SDIO_STD_CAPACITY_SD_CARD_V1_1      ((uint32_t)0U)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0      ((uint32_t)1U)
#define SDIO_HIGH_CAPACITY_SD_CARD          ((uint32_t)2U)
#define SDIO_MULTIMEDIA_CARD                ((uint32_t)3U)
#define SDIO_SECURE_DIGITAL_IO_CARD         ((uint32_t)4U)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD   ((uint32_t)5U)
#endif

/* ==================== R1 响应错误位 ==================== */
#define SD_R1_OUT_OF_RANGE                  BIT(31)
#define SD_R1_ADDRESS_ERROR                 BIT(30)
#define SD_R1_BLOCK_LEN_ERROR               BIT(29)
#define SD_R1_ERASE_SEQ_ERROR               BIT(28)
#define SD_R1_ERASE_PARAM                   BIT(27)
#define SD_R1_WP_VIOLATION                  BIT(26)
#define SD_R1_CARD_IS_LOCKED                BIT(25)
#define SD_R1_LOCK_UNLOCK_FAILED            BIT(24)
#define SD_R1_COM_CRC_ERROR                 BIT(23)
#define SD_R1_ILLEGAL_COMMAND               BIT(22)
#define SD_R1_CARD_ECC_FAILED               BIT(21)
#define SD_R1_CC_ERROR                      BIT(20)
#define SD_R1_GENERAL_UNKNOWN_ERROR         BIT(19)
#define SD_R1_CSD_OVERWRITE                 BIT(16)
#define SD_R1_WP_ERASE_SKIP                 BIT(15)
#define SD_R1_CARD_ECC_DISABLED             BIT(14)
#define SD_R1_ERASE_RESET                   BIT(13)
#define SD_R1_READY_FOR_DATA                BIT(8)
#define SD_R1_APP_CMD                       BIT(5)
#define SD_R1_AKE_SEQ_ERROR                 BIT(3)
#define SD_R1_ERROR_BITS                    ((uint32_t)0xFDF9E008)

/* ==================== R6 响应错误位 ==================== */
#define SD_R6_COM_CRC_ERROR                 BIT(15)
#define SD_R6_ILLEGAL_COMMAND               BIT(14)
#define SD_R6_GENERAL_UNKNOWN_ERROR         BIT(13)

/* ==================== 卡状态 ==================== */
#define SD_CARDSTATE_IDLE                   ((uint8_t)0x00)
#define SD_CARDSTATE_READY                  ((uint8_t)0x01)
#define SD_CARDSTATE_IDENTIFICAT            ((uint8_t)0x02)
#define SD_CARDSTATE_STANDBY                ((uint8_t)0x03)
#define SD_CARDSTATE_TRANSFER               ((uint8_t)0x04)
#define SD_CARDSTATE_DATA                   ((uint8_t)0x05)
#define SD_CARDSTATE_RECEIVING              ((uint8_t)0x06)
#define SD_CARDSTATE_PROGRAMMING            ((uint8_t)0x07)
#define SD_CARDSTATE_DISCONNECT             ((uint8_t)0x08)
#define SD_CARDSTATE_LOCKED                 ((uint32_t)0x02000000)

/* ==================== 协议常量 ==================== */
#define SD_CHECK_PATTERN                    ((uint32_t)0x000001AA)
#define SD_VOLTAGE_WINDOW                   ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                    ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                     ((uint32_t)0x00000000)

#define SD_BUS_WIDTH_4BIT                   ((uint32_t)0x00040000)
#define SD_BUS_WIDTH_1BIT                   ((uint32_t)0x00010000)

#define SD_MASK_0_7BITS                     ((uint32_t)0x000000FF)
#define SD_MASK_8_15BITS                    ((uint32_t)0x0000FF00)
#define SD_MASK_16_23BITS                   ((uint32_t)0x00FF0000)
#define SD_MASK_24_31BITS                   ((uint32_t)0xFF000000)

#define SD_FIFOHALF_WORDS                   ((uint32_t)0x00000008)
#define SD_FIFOHALF_BYTES                   ((uint32_t)0x00000020)

#define SD_DATATIMEOUT                      ((uint32_t)0xFFFFFFFF)
#define SD_MAX_VOLT_VALIDATION              ((uint32_t)0x0000FFFF)
#define SD_MAX_DATA_LENGTH                  ((uint32_t)0x01FFFFFF)
#define SD_ALLZERO                          ((uint32_t)0x00000000)
#define SD_RCA_SHIFT                        ((uint8_t)0x10)
#define SD_CLK_DIV_INIT                     ((uint16_t)0x01F6)  /* APB2/504，确保识别阶段 ≤400kHz */
#define SD_CLK_DIV_TRANS                    ((uint16_t)0x0002)

#define SDIO_MASK_INTC_FLAGS                ((uint32_t)0x00C007FF)

/* ==================== 全局/静态变量 ==================== */
uint32_t sd_scr[2] = {0, 0};

static uint32_t cardtype = SDIO_STD_CAPACITY_SD_CARD_V1_1;
static uint32_t sd_csd[4] = {0, 0, 0, 0};
static uint32_t sd_cid[4] = {0, 0, 0, 0};
static uint16_t sd_rca = 0;
static uint32_t transmode = SD_POLLING_MODE;
static uint32_t totalnumber_bytes = 0, stopcondition = 0;
static __IO sd_error_typedef_enum transerror = SD_OK;
static __IO uint32_t transend = 0, number_bytes = 0;

/* ==================== 静态函数声明 ==================== */
static sd_error_typedef_enum cmdsent_error_check(void);
static sd_error_typedef_enum r1_error_check(uint8_t cmdindex);
static sd_error_typedef_enum r1_error_type_check(uint32_t resp);
static sd_error_typedef_enum r2_error_check(void);
static sd_error_typedef_enum r3_error_check(void);
static sd_error_typedef_enum r6_error_check(uint8_t cmdindex, uint16_t *prca);
static sd_error_typedef_enum r7_error_check(void);

static sd_error_typedef_enum sd_card_state_get(uint8_t *pcardstate);
static sd_error_typedef_enum sd_bus_width_config(uint32_t buswidth);
static sd_error_typedef_enum sd_scr_get(uint16_t rca, uint32_t *pscr);
static uint32_t sd_datablocksize_get(uint16_t bytesnumber);

/*!
    \brief      初始化 SD 卡，进入 standby 状态
*/
sd_error_typedef_enum sd_init(void)
{
    sd_error_typedef_enum status;

    bsp_sdio_init();
    status = sd_power_on();
    if(SD_OK != status) {
        return status;
    }

    status = sd_card_init();
    if(SD_OK != status) {
        return status;
    }

    status = sd_scr_get(sd_rca, sd_scr);
    if(SD_OK != status) {
        return status;
    }

    bsp_sdio_clock_set(SD_CLK_DIV_TRANS, SDIO_BUSMODE_1BIT);
    return status;
}

/*!
    \brief      初始化卡，读取 CID 与 CSD
*/
sd_error_typedef_enum sd_card_init(void)
{
    sd_error_typedef_enum status = SD_OK;
    uint16_t temp_rca = 0x01;

    if(SDIO_POWER_OFF == sdio_power_state_get()) {
        return SD_OPERATION_IMPROPER;
    }

    if(SDIO_SECURE_DIGITAL_IO_CARD != cardtype) {
        bsp_sdio_send_cmd(SD_CMD_ALL_SEND_CID, 0, BSP_SDIO_RESP_LONG);
        status = r2_error_check();
        if(SD_OK != status) {
            return status;
        }
        sd_cid[0] = bsp_sdio_get_response(0);
        sd_cid[1] = bsp_sdio_get_response(1);
        sd_cid[2] = bsp_sdio_get_response(2);
        sd_cid[3] = bsp_sdio_get_response(3);
    }

    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
       (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype) ||
       (SDIO_HIGH_CAPACITY_SD_CARD == cardtype) ||
       (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == cardtype)) {
        bsp_sdio_send_cmd(SD_CMD_SEND_RELATIVE_ADDR, 0, BSP_SDIO_RESP_SHORT);
        status = r6_error_check(SD_CMD_SEND_RELATIVE_ADDR, &temp_rca);
        if(SD_OK != status) {
            return status;
        }
    }

    if(SDIO_SECURE_DIGITAL_IO_CARD != cardtype) {
        sd_rca = temp_rca;

        bsp_sdio_send_cmd(SD_CMD_SEND_CSD,
                          (uint32_t)(temp_rca << SD_RCA_SHIFT),
                          BSP_SDIO_RESP_LONG);
        status = r2_error_check();
        if(SD_OK != status) {
            return status;
        }
        sd_csd[0] = bsp_sdio_get_response(0);
        sd_csd[1] = bsp_sdio_get_response(1);
        sd_csd[2] = bsp_sdio_get_response(2);
        sd_csd[3] = bsp_sdio_get_response(3);
    }
    return status;
}

/*!
    \brief      上电并识别卡类型
*/
sd_error_typedef_enum sd_power_on(void)
{
    sd_error_typedef_enum status = SD_OK;
    uint32_t sdcardtype = SD_STD_CAPACITY, response = 0, count = 0;
    uint8_t busyflag = 0;

    sdio_clock_config(SDIO_SDIOCLKEDGE_RISING, SDIO_CLOCKBYPASS_DISABLE,
                      SDIO_CLOCKPWRSAVE_DISABLE, SD_CLK_DIV_INIT);
    sdio_bus_mode_set(SDIO_BUSMODE_1BIT);
    sdio_hardware_clock_disable();
    sdio_power_state_set(SDIO_POWER_ON);
    sdio_clock_enable();

    bsp_sdio_send_cmd(SD_CMD_GO_IDLE_STATE, 0, BSP_SDIO_RESP_NONE);
    status = cmdsent_error_check();
    if(SD_OK != status) {
        return status;
    }

    bsp_sdio_send_cmd(SD_CMD_SEND_IF_COND, SD_CHECK_PATTERN, BSP_SDIO_RESP_SHORT);
    {
        sd_error_typedef_enum r7 = r7_error_check();
        uint32_t r7_resp = bsp_sdio_get_response(0);
        printf("[SD] CMD8 r7=%d resp=0x%08X\r\n", r7, r7_resp);
        if(SD_OK == r7) {
            cardtype = SDIO_STD_CAPACITY_SD_CARD_V2_0;
            sdcardtype = SD_HIGH_CAPACITY;
        }
    }

    bsp_sdio_send_cmd(SD_CMD_APP_CMD, 0, BSP_SDIO_RESP_SHORT);
    if(SD_OK == r1_error_check(SD_CMD_APP_CMD)) {
		while((!busyflag) && (count < 200U)) {//while((!busyflag) && (count < SD_MAX_VOLT_VALIDATION)) {
            bsp_sdio_send_cmd(SD_CMD_APP_CMD, 0, BSP_SDIO_RESP_SHORT);
            status = r1_error_check(SD_CMD_APP_CMD);
            if(SD_OK != status) {
                return status;
            }

            bsp_sdio_send_cmd(SD_APPCMD_SD_SEND_OP_COND,
                              (SD_VOLTAGE_WINDOW | sdcardtype),
                              BSP_SDIO_RESP_SHORT);
            status = r3_error_check();
            if(SD_OK != status) {
                return status;
            }
            response = bsp_sdio_get_response(0);
            busyflag = (uint8_t)((response >> 31) & 0x01);
            printf("[SD] ACMD41 cnt=%u resp=0x%08X busy=%u\r\n", count, response, busyflag);
            ++count;
        }
        if(count >= 200U){//if(count >= SD_MAX_VOLT_VALIDATION) {
            return SD_VOLTRANGE_INVALID;
        }
        if(response & SD_HIGH_CAPACITY) {
            cardtype = SDIO_HIGH_CAPACITY_SD_CARD;
        }
    }
    return status;
}

/*!
    \brief      关闭 SDIO 电源
*/
sd_error_typedef_enum sd_power_off(void)
{
    sdio_power_state_set(SDIO_POWER_OFF);
    return SD_OK;
}

/*!
    \brief      配置总线模式（1bit / 4bit）
*/
sd_error_typedef_enum sd_bus_mode_config(uint32_t busmode)
{
    sd_error_typedef_enum status = SD_OK;

    if(SDIO_MULTIMEDIA_CARD == cardtype) {
        return SD_FUNCTION_UNSUPPORTED;
    } else if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
              (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype) ||
              (SDIO_HIGH_CAPACITY_SD_CARD == cardtype)) {
        if(SDIO_BUSMODE_8BIT == busmode) {
            return SD_FUNCTION_UNSUPPORTED;
        } else if(SDIO_BUSMODE_4BIT == busmode) {
            status = sd_bus_width_config(SD_BUS_WIDTH_4BIT);
            if(SD_OK == status) {
                bsp_sdio_clock_set(SD_CLK_DIV_TRANS, SDIO_BUSMODE_4BIT);
            }
        } else if(SDIO_BUSMODE_1BIT == busmode) {
            status = sd_bus_width_config(SD_BUS_WIDTH_1BIT);
            if(SD_OK == status) {
                bsp_sdio_clock_set(SD_CLK_DIV_TRANS, SDIO_BUSMODE_1BIT);
            }
        } else {
            status = SD_PARAMETER_INVALID;
        }
    }
    return status;
}

/*!
    \brief      配置传输模式（DMA / 轮询）
*/
sd_error_typedef_enum sd_transfer_mode_config(uint32_t txmode)
{
    if((SD_DMA_MODE == txmode) || (SD_POLLING_MODE == txmode)) {
        transmode = txmode;
        return SD_OK;
    }
    return SD_PARAMETER_INVALID;
}

/*!
    \brief      单块读
*/
sd_error_typedef_enum sd_block_read(uint32_t *preadbuffer, uint32_t readaddr, uint16_t blocksize)
{
    sd_error_typedef_enum status = SD_OK;
    uint32_t count = 0, align = 0;
    uint32_t datablksize = SDIO_DATABLOCKSIZE_1BYTE, *ptempbuff = preadbuffer;
    __IO uint32_t timeout = 0;

    if(NULL == preadbuffer) {
        return SD_PARAMETER_INVALID;
    }

    transerror = SD_OK;
    transend = 0;
    totalnumber_bytes = 0;
    sdio_data_config(0, 0, SDIO_DATABLOCKSIZE_1BYTE);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_disable();
    sdio_dma_disable();

    if(bsp_sdio_get_response(0) & SD_CARDSTATE_LOCKED) {
        return SD_LOCK_UNLOCK_FAILED;
    }

    if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        blocksize = 512;
        readaddr /= 512;
    }

    align = blocksize & (blocksize - 1);
    if((blocksize > 0) && (blocksize <= 2048) && (0 == align)) {
        datablksize = sd_datablocksize_get(blocksize);
        bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, blocksize, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_SET_BLOCKLEN);
        if(SD_OK != status) {
            return status;
        }
    } else {
        return SD_PARAMETER_INVALID;
    }

    stopcondition = 0;
    totalnumber_bytes = blocksize;

    sdio_data_config(SD_DATATIMEOUT, totalnumber_bytes, datablksize);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOSDIO);
    sdio_dsm_enable();

    bsp_sdio_send_cmd(SD_CMD_READ_SINGLE_BLOCK, readaddr, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_READ_SINGLE_BLOCK);
    if(SD_OK != status) {
        return status;
    }

    if(SD_POLLING_MODE == transmode) {
        while(!sdio_flag_get(SDIO_FLAG_DTCRCERR | SDIO_FLAG_DTTMOUT |
                             SDIO_FLAG_RXORE | SDIO_FLAG_DTBLKEND | SDIO_FLAG_STBITE)) {
            if(RESET != sdio_flag_get(SDIO_FLAG_RFH)) {
                for(count = 0; count < SD_FIFOHALF_WORDS; count++) {
                    *(ptempbuff + count) = bsp_sdio_read_word();
                }
                ptempbuff += SD_FIFOHALF_WORDS;
            }
        }
        if(RESET != sdio_flag_get(SDIO_FLAG_DTCRCERR)) {
            sdio_flag_clear(SDIO_FLAG_DTCRCERR);
            return SD_DATA_CRC_ERROR;
        } else if(RESET != sdio_flag_get(SDIO_FLAG_DTTMOUT)) {
            sdio_flag_clear(SDIO_FLAG_DTTMOUT);
            return SD_DATA_TIMEOUT;
        } else if(RESET != sdio_flag_get(SDIO_FLAG_RXORE)) {
            sdio_flag_clear(SDIO_FLAG_RXORE);
            return SD_RX_OVERRUN_ERROR;
        } else if((RESET != sdio_flag_get(SDIO_FLAG_STBITE)) &&
                  (RESET == sdio_flag_get(SDIO_FLAG_DTEND))) {
            sdio_flag_clear(SDIO_FLAG_STBITE);
            return SD_START_BIT_ERROR;
        }
        while(RESET != sdio_flag_get(SDIO_FLAG_RXDTVAL)) {
            *ptempbuff = bsp_sdio_read_word();
            ++ptempbuff;
        }
        sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    } else if(SD_DMA_MODE == transmode) {
        sdio_interrupt_enable(SDIO_INT_CCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_RXORE |
                              SDIO_INT_DTEND | SDIO_INT_STBITE);
        sdio_dma_enable();
        bsp_sdio_dma_rx_config(preadbuffer, blocksize);

        timeout = 100000;
        while((0 == bsp_sdio_dma_is_done()) && (timeout > 0)) {
            timeout--;
            if(0 == timeout) {
                return SD_ERROR;
            }
        }
    } else {
        return SD_PARAMETER_INVALID;
    }
    return status;
}

/*!
    \brief      多块读
*/
sd_error_typedef_enum sd_multiblocks_read(uint32_t *preadbuffer, uint32_t readaddr,
                                          uint16_t blocksize, uint32_t blocksnumber)
{
    sd_error_typedef_enum status = SD_OK;
    uint32_t count = 0, align = 0;
    uint32_t datablksize = SDIO_DATABLOCKSIZE_1BYTE, *ptempbuff = preadbuffer;
    __IO uint32_t timeout = 0;

    if(NULL == preadbuffer) {
        return SD_PARAMETER_INVALID;
    }

    transerror = SD_OK;
    transend = 0;
    totalnumber_bytes = 0;
    sdio_data_config(0, 0, SDIO_DATABLOCKSIZE_1BYTE);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_disable();
    sdio_dma_disable();

    if(bsp_sdio_get_response(0) & SD_CARDSTATE_LOCKED) {
        return SD_LOCK_UNLOCK_FAILED;
    }

    if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        blocksize = 512;
        readaddr /= 512;
    }

    align = blocksize & (blocksize - 1);
    if((blocksize > 0) && (blocksize <= 2048) && (0 == align)) {
        datablksize = sd_datablocksize_get(blocksize);
        bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, blocksize, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_SET_BLOCKLEN);
        if(SD_OK != status) {
            return status;
        }
    } else {
        return SD_PARAMETER_INVALID;
    }

    if(blocksnumber > 1) {
        if(blocksnumber * blocksize > SD_MAX_DATA_LENGTH) {
            return SD_PARAMETER_INVALID;
        }

        stopcondition = 1;
        totalnumber_bytes = blocksnumber * blocksize;

        sdio_data_config(SD_DATATIMEOUT, totalnumber_bytes, datablksize);
        sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOSDIO);
        sdio_dsm_enable();

        bsp_sdio_send_cmd(SD_CMD_READ_MULTIPLE_BLOCK, readaddr, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_READ_MULTIPLE_BLOCK);
        if(SD_OK != status) {
            return status;
        }

        if(SD_POLLING_MODE == transmode) {
            while(!sdio_flag_get(SDIO_FLAG_DTCRCERR | SDIO_FLAG_DTTMOUT |
                                 SDIO_FLAG_RXORE | SDIO_FLAG_DTEND | SDIO_FLAG_STBITE)) {
                if(RESET != sdio_flag_get(SDIO_FLAG_RFH)) {
                    for(count = 0; count < SD_FIFOHALF_WORDS; count++) {
                        *(ptempbuff + count) = bsp_sdio_read_word();
                    }
                    ptempbuff += SD_FIFOHALF_WORDS;
                }
            }
            if(RESET != sdio_flag_get(SDIO_FLAG_DTCRCERR)) {
                sdio_flag_clear(SDIO_FLAG_DTCRCERR);
                return SD_DATA_CRC_ERROR;
            } else if(RESET != sdio_flag_get(SDIO_FLAG_DTTMOUT)) {
                sdio_flag_clear(SDIO_FLAG_DTTMOUT);
                return SD_DATA_TIMEOUT;
            } else if(RESET != sdio_flag_get(SDIO_FLAG_RXORE)) {
                sdio_flag_clear(SDIO_FLAG_RXORE);
                return SD_RX_OVERRUN_ERROR;
            } else if((RESET != sdio_flag_get(SDIO_FLAG_STBITE)) &&
                      (RESET == sdio_flag_get(SDIO_FLAG_DTEND))) {
                sdio_flag_clear(SDIO_FLAG_STBITE);
                return SD_START_BIT_ERROR;
            }
            while(RESET != sdio_flag_get(SDIO_FLAG_RXDTVAL)) {
                *ptempbuff = bsp_sdio_read_word();
                ++ptempbuff;
            }

            if(RESET != sdio_flag_get(SDIO_FLAG_DTEND)) {
                if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
                   (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype) ||
                   (SDIO_HIGH_CAPACITY_SD_CARD == cardtype)) {
                    bsp_sdio_send_cmd(SD_CMD_STOP_TRANSMISSION, 0, BSP_SDIO_RESP_SHORT);
                    status = r1_error_check(SD_CMD_STOP_TRANSMISSION);
                    if(SD_OK != status) {
                        return status;
                    }
                }
            }
            sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
        } else if(SD_DMA_MODE == transmode) {
            sdio_interrupt_enable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT |
                                  SDIO_INT_RXORE | SDIO_INT_DTEND | SDIO_INT_STBITE);
            sdio_dma_enable();
            bsp_sdio_dma_rx_config(preadbuffer, totalnumber_bytes);

            timeout = 100000;
            while((0 == bsp_sdio_dma_is_done()) && (timeout > 0)) {
                timeout--;
                if(0 == timeout) {
                    return SD_ERROR;
                }
            }
            while((0 == transend) && (SD_OK == transerror)) {
            }
            if(SD_OK != transerror) {
                return transerror;
            }
        } else {
            return SD_PARAMETER_INVALID;
        }
    }
    return status;
}

/*!
    \brief      单块写
*/
sd_error_typedef_enum sd_block_write(uint32_t *pwritebuffer, uint32_t writeaddr, uint16_t blocksize)
{
    sd_error_typedef_enum status = SD_OK;
    uint8_t cardstate = 0;
    uint32_t count = 0, align = 0;
    uint32_t datablksize = SDIO_DATABLOCKSIZE_1BYTE, *ptempbuff = pwritebuffer;
    uint32_t transbytes = 0, restwords = 0, response = 0;
    __IO uint32_t timeout = 0;

    if(NULL == pwritebuffer) {
        return SD_PARAMETER_INVALID;
    }

    transerror = SD_OK;
    transend = 0;
    totalnumber_bytes = 0;
    sdio_data_config(0, 0, SDIO_DATABLOCKSIZE_1BYTE);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_disable();
    sdio_dma_disable();

    if(bsp_sdio_get_response(0) & SD_CARDSTATE_LOCKED) {
        return SD_LOCK_UNLOCK_FAILED;
    }

    if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        blocksize = 512;
        writeaddr /= 512;
    }

    align = blocksize & (blocksize - 1);
    if((blocksize > 0) && (blocksize <= 2048) && (0 == align)) {
        datablksize = sd_datablocksize_get(blocksize);
        bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, blocksize, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_SET_BLOCKLEN);
        if(SD_OK != status) {
            return status;
        }
    } else {
        return SD_PARAMETER_INVALID;
    }

    bsp_sdio_send_cmd(SD_CMD_SEND_STATUS, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SEND_STATUS);
    if(SD_OK != status) {
        return status;
    }

    response = bsp_sdio_get_response(0);
    timeout = 100000;
    while((0 == (response & SD_R1_READY_FOR_DATA)) && (timeout > 0)) {
        --timeout;
        bsp_sdio_send_cmd(SD_CMD_SEND_STATUS, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_SEND_STATUS);
        if(SD_OK != status) {
            return status;
        }
        response = bsp_sdio_get_response(0);
    }
    if(0 == timeout) {
        return SD_ERROR;
    }

    bsp_sdio_send_cmd(SD_CMD_WRITE_BLOCK, writeaddr, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_WRITE_BLOCK);
    if(SD_OK != status) {
        return status;
    }

    stopcondition = 0;
    totalnumber_bytes = blocksize;

    sdio_data_config(SD_DATATIMEOUT, totalnumber_bytes, datablksize);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_enable();

    if(SD_POLLING_MODE == transmode) {
        while(!sdio_flag_get(SDIO_FLAG_DTCRCERR | SDIO_FLAG_DTTMOUT |
                             SDIO_FLAG_TXURE | SDIO_FLAG_DTBLKEND | SDIO_FLAG_STBITE)) {
            if(RESET != sdio_flag_get(SDIO_FLAG_TFH)) {
                if((totalnumber_bytes - transbytes) < SD_FIFOHALF_BYTES) {
                    restwords = (totalnumber_bytes - transbytes) / 4 +
                                (((totalnumber_bytes - transbytes) % 4 == 0) ? 0 : 1);
                    for(count = 0; count < restwords; count++) {
                        bsp_sdio_write_word(*ptempbuff);
                        ++ptempbuff;
                        transbytes += 4;
                    }
                } else {
                    for(count = 0; count < SD_FIFOHALF_WORDS; count++) {
                        bsp_sdio_write_word(*(ptempbuff + count));
                    }
                    ptempbuff += SD_FIFOHALF_WORDS;
                    transbytes += SD_FIFOHALF_BYTES;
                }
            }
        }
        if(RESET != sdio_flag_get(SDIO_FLAG_DTCRCERR)) {
            sdio_flag_clear(SDIO_FLAG_DTCRCERR);
            return SD_DATA_CRC_ERROR;
        } else if(RESET != sdio_flag_get(SDIO_FLAG_DTTMOUT)) {
            sdio_flag_clear(SDIO_FLAG_DTTMOUT);
            return SD_DATA_TIMEOUT;
        } else if(RESET != sdio_flag_get(SDIO_FLAG_TXURE)) {
            sdio_flag_clear(SDIO_FLAG_TXURE);
            return SD_TX_UNDERRUN_ERROR;
        } else if(RESET != sdio_flag_get(SDIO_FLAG_STBITE)) {
            sdio_flag_clear(SDIO_FLAG_STBITE);
            return SD_START_BIT_ERROR;
        }
    } else if(SD_DMA_MODE == transmode) {
        sdio_interrupt_enable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT |
                              SDIO_INT_TXURE | SDIO_INT_DTEND | SDIO_INT_STBITE);
        bsp_sdio_dma_tx_config(pwritebuffer, blocksize);
        sdio_dma_enable();

        timeout = 100000;
        while((0 == bsp_sdio_dma_is_done()) && (timeout > 0)) {
            timeout--;
            if(0 == timeout) {
                return SD_ERROR;
            }
        }
        while((0 == transend) && (SD_OK == transerror)) {
        }
        if(SD_OK != transerror) {
            return transerror;
        }
    } else {
        return SD_PARAMETER_INVALID;
    }

    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    status = sd_card_state_get(&cardstate);
    while((SD_OK == status) &&
          ((SD_CARDSTATE_PROGRAMMING == cardstate) || (SD_CARDSTATE_RECEIVING == cardstate))) {
        status = sd_card_state_get(&cardstate);
    }
    return status;
}

/*!
    \brief      多块写
*/
sd_error_typedef_enum sd_multiblocks_write(uint32_t *pwritebuffer, uint32_t writeaddr,
                                           uint16_t blocksize, uint32_t blocksnumber)
{
    sd_error_typedef_enum status = SD_OK;
    uint8_t cardstate = 0;
    uint32_t count = 0, align = 0;
    uint32_t datablksize = SDIO_DATABLOCKSIZE_1BYTE, *ptempbuff = pwritebuffer;
    uint32_t transbytes = 0, restwords = 0;
    __IO uint32_t timeout = 0;

    if(NULL == pwritebuffer) {
        return SD_PARAMETER_INVALID;
    }

    transerror = SD_OK;
    transend = 0;
    totalnumber_bytes = 0;
    sdio_data_config(0, 0, SDIO_DATABLOCKSIZE_1BYTE);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_disable();
    sdio_dma_disable();

    if(bsp_sdio_get_response(0) & SD_CARDSTATE_LOCKED) {
        return SD_LOCK_UNLOCK_FAILED;
    }

    if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        blocksize = 512;
        writeaddr /= 512;
    }

    align = blocksize & (blocksize - 1);
    if((blocksize > 0) && (blocksize <= 2048) && (0 == align)) {
        datablksize = sd_datablocksize_get(blocksize);
        bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, blocksize, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_SET_BLOCKLEN);
        if(SD_OK != status) {
            return status;
        }
    } else {
        return SD_PARAMETER_INVALID;
    }

    bsp_sdio_send_cmd(SD_CMD_SEND_STATUS, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SEND_STATUS);
    if(SD_OK != status) {
        return status;
    }

    if(blocksnumber > 1) {
        if(blocksnumber * blocksize > SD_MAX_DATA_LENGTH) {
            return SD_PARAMETER_INVALID;
        }

        if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
           (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype) ||
           (SDIO_HIGH_CAPACITY_SD_CARD == cardtype)) {
            bsp_sdio_send_cmd(SD_CMD_APP_CMD, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
            status = r1_error_check(SD_CMD_APP_CMD);
            if(SD_OK != status) {
                return status;
            }
            bsp_sdio_send_cmd(SD_APPCMD_SET_WR_BLK_ERASE_COUNT, blocksnumber, BSP_SDIO_RESP_SHORT);
            status = r1_error_check(SD_APPCMD_SET_WR_BLK_ERASE_COUNT);
            if(SD_OK != status) {
                return status;
            }
        }
        bsp_sdio_send_cmd(SD_CMD_WRITE_MULTIPLE_BLOCK, writeaddr, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_WRITE_MULTIPLE_BLOCK);
        if(SD_OK != status) {
            return status;
        }

        stopcondition = 1;
        totalnumber_bytes = blocksnumber * blocksize;

        sdio_data_config(SD_DATATIMEOUT, totalnumber_bytes, datablksize);
        sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
        sdio_dsm_enable();

        if(SD_POLLING_MODE == transmode) {
            while(!sdio_flag_get(SDIO_FLAG_DTCRCERR | SDIO_FLAG_DTTMOUT |
                                 SDIO_FLAG_TXURE | SDIO_FLAG_DTEND | SDIO_FLAG_STBITE)) {
                if(RESET != sdio_flag_get(SDIO_FLAG_TFH)) {
                    if(!((totalnumber_bytes - transbytes) < SD_FIFOHALF_BYTES)) {
                        for(count = 0; count < SD_FIFOHALF_WORDS; count++) {
                            bsp_sdio_write_word(*(ptempbuff + count));
                        }
                        ptempbuff += SD_FIFOHALF_WORDS;
                        transbytes += SD_FIFOHALF_BYTES;
                    } else {
                        restwords = (totalnumber_bytes - transbytes) / 4 +
                                    (((totalnumber_bytes - transbytes) % 4 == 0) ? 0 : 1);
                        for(count = 0; count < restwords; count++) {
                            bsp_sdio_write_word(*ptempbuff);
                            ++ptempbuff;
                            transbytes += 4;
                        }
                    }
                }
            }
            if(RESET != sdio_flag_get(SDIO_FLAG_DTCRCERR)) {
                sdio_flag_clear(SDIO_FLAG_DTCRCERR);
                return SD_DATA_CRC_ERROR;
            } else if(RESET != sdio_flag_get(SDIO_FLAG_DTTMOUT)) {
                sdio_flag_clear(SDIO_FLAG_DTTMOUT);
                return SD_DATA_TIMEOUT;
            } else if(RESET != sdio_flag_get(SDIO_FLAG_TXURE)) {
                sdio_flag_clear(SDIO_FLAG_TXURE);
                return SD_TX_UNDERRUN_ERROR;
            } else if(RESET != sdio_flag_get(SDIO_FLAG_STBITE)) {
                sdio_flag_clear(SDIO_FLAG_STBITE);
                return SD_START_BIT_ERROR;
            }

            if(RESET != sdio_flag_get(SDIO_FLAG_DTEND)) {
                if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
                   (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype) ||
                   (SDIO_HIGH_CAPACITY_SD_CARD == cardtype)) {
                    bsp_sdio_send_cmd(SD_CMD_STOP_TRANSMISSION, 0, BSP_SDIO_RESP_SHORT);
                    status = r1_error_check(SD_CMD_STOP_TRANSMISSION);
                    if(SD_OK != status) {
                        return status;
                    }
                }
            }
            sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
        } else if(SD_DMA_MODE == transmode) {
            sdio_interrupt_enable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT |
                                  SDIO_INT_TXURE | SDIO_INT_DTEND | SDIO_INT_STBITE);
            sdio_dma_enable();
            bsp_sdio_dma_tx_config(pwritebuffer, totalnumber_bytes);

            timeout = 200000;
            while((0 == bsp_sdio_dma_is_done()) && (timeout > 0)) {
                timeout--;
                if(0 == timeout) {
                    return SD_ERROR;
                }
            }
            while((0 == transend) && (SD_OK == transerror)) {
            }
            if(SD_OK != transerror) {
                return transerror;
            }
        } else {
            return SD_PARAMETER_INVALID;
        }
    }

    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    status = sd_card_state_get(&cardstate);
    while((SD_OK == status) &&
          ((SD_CARDSTATE_PROGRAMMING == cardstate) || (SD_CARDSTATE_RECEIVING == cardstate))) {
        status = sd_card_state_get(&cardstate);
    }
    return status;
}

/*!
    \brief      擦除连续区域
*/
sd_error_typedef_enum sd_erase(uint32_t startaddr, uint32_t endaddr)
{
    sd_error_typedef_enum status = SD_OK;
    uint32_t count = 0, clkdiv = 0;
    __IO uint32_t delay = 0;
    uint8_t cardstate = 0, tempbyte = 0;
    uint16_t tempccc = 0;

    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_24_31BITS) >> 24);
    tempccc = (uint16_t)((uint16_t)tempbyte << 4);
    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_16_23BITS) >> 16);
    tempccc |= (uint16_t)((uint16_t)(tempbyte & 0xF0) >> 4);
    if(0 == (tempccc & SD_CCC_ERASE)) {
        return SD_FUNCTION_UNSUPPORTED;
    }

    clkdiv = (SDIO_CLKCTL & SDIO_CLKCTL_DIV);
    clkdiv += ((SDIO_CLKCTL & SDIO_CLKCTL_DIV8) >> 31) * 256;
    clkdiv += 2;
    delay = 168000 / clkdiv;

    if(bsp_sdio_get_response(0) & SD_CARDSTATE_LOCKED) {
        return SD_LOCK_UNLOCK_FAILED;
    }

    if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        startaddr /= 512;
        endaddr /= 512;
    }

    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
       (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype) ||
       (SDIO_HIGH_CAPACITY_SD_CARD == cardtype)) {
        bsp_sdio_send_cmd(SD_CMD_ERASE_WR_BLK_START, startaddr, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_ERASE_WR_BLK_START);
        if(SD_OK != status) {
            return status;
        }
        bsp_sdio_send_cmd(SD_CMD_ERASE_WR_BLK_END, endaddr, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_ERASE_WR_BLK_END);
        if(SD_OK != status) {
            return status;
        }
    }

    bsp_sdio_send_cmd(SD_CMD_ERASE, 0, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_ERASE);
    if(SD_OK != status) {
        return status;
    }

    for(count = 0; count < delay; count++) {
    }

    status = sd_card_state_get(&cardstate);
    while((SD_OK == status) &&
          ((SD_CARDSTATE_PROGRAMMING == cardstate) || (SD_CARDSTATE_RECEIVING == cardstate))) {
        status = sd_card_state_get(&cardstate);
    }
    return status;
}

/*!
    \brief      SDIO 中断处理
*/
sd_error_typedef_enum sd_interrupts_process(void)
{
    transerror = SD_OK;

    if(RESET != sdio_interrupt_flag_get(SDIO_INT_FLAG_DTEND)) {
        if(1 == stopcondition) {
            transerror = sd_transfer_stop();
        } else {
            transerror = SD_OK;
        }
        sdio_interrupt_flag_clear(SDIO_INT_DTEND);
        sdio_interrupt_disable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_DTEND |
                               SDIO_INT_STBITE | SDIO_INT_TFH | SDIO_INT_RFH |
                               SDIO_INT_TXURE | SDIO_INT_RXORE);
        transend = 1;
        number_bytes = 0;
        return transerror;
    }

    if(RESET != sdio_interrupt_flag_get(SDIO_INT_FLAG_DTCRCERR)) {
        sdio_interrupt_flag_clear(SDIO_INT_DTCRCERR);
        sdio_interrupt_disable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_DTEND |
                               SDIO_INT_STBITE | SDIO_INT_TFH | SDIO_INT_RFH |
                               SDIO_INT_TXURE | SDIO_INT_RXORE);
        number_bytes = 0;
        transerror = SD_DATA_CRC_ERROR;
        return transerror;
    }

    if(RESET != sdio_interrupt_flag_get(SDIO_INT_FLAG_DTTMOUT)) {
        sdio_interrupt_flag_clear(SDIO_INT_DTTMOUT);
        sdio_interrupt_disable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_DTEND |
                               SDIO_INT_STBITE | SDIO_INT_TFH | SDIO_INT_RFH |
                               SDIO_INT_TXURE | SDIO_INT_RXORE);
        number_bytes = 0;
        transerror = SD_DATA_TIMEOUT;
        return transerror;
    }

    if(RESET != sdio_interrupt_flag_get(SDIO_INT_FLAG_STBITE)) {
        sdio_interrupt_flag_clear(SDIO_INT_STBITE);
        sdio_interrupt_disable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_DTEND |
                               SDIO_INT_STBITE | SDIO_INT_TFH | SDIO_INT_RFH |
                               SDIO_INT_TXURE | SDIO_INT_RXORE);
        number_bytes = 0;
        transerror = SD_START_BIT_ERROR;
        return transerror;
    }

    if(RESET != sdio_interrupt_flag_get(SDIO_INT_FLAG_TXURE)) {
        sdio_interrupt_flag_clear(SDIO_INT_TXURE);
        sdio_interrupt_disable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_DTEND |
                               SDIO_INT_STBITE | SDIO_INT_TFH | SDIO_INT_RFH |
                               SDIO_INT_TXURE | SDIO_INT_RXORE);
        number_bytes = 0;
        transerror = SD_TX_UNDERRUN_ERROR;
        return transerror;
    }

    if(RESET != sdio_interrupt_flag_get(SDIO_INT_FLAG_RXORE)) {
        sdio_interrupt_flag_clear(SDIO_INT_RXORE);
        sdio_interrupt_disable(SDIO_INT_DTCRCERR | SDIO_INT_DTTMOUT | SDIO_INT_DTEND |
                               SDIO_INT_STBITE | SDIO_INT_TFH | SDIO_INT_RFH |
                               SDIO_INT_TXURE | SDIO_INT_RXORE);
        number_bytes = 0;
        transerror = SD_RX_OVERRUN_ERROR;
        return transerror;
    }
    return transerror;
}

/*!
    \brief      选择/取消选择卡
*/
sd_error_typedef_enum sd_card_select_deselect(uint16_t cardrca)
{
    bsp_sdio_send_cmd(SD_CMD_SELECT_DESELECT_CARD,
                      (uint32_t)(cardrca << SD_RCA_SHIFT),
                      BSP_SDIO_RESP_SHORT);
    return r1_error_check(SD_CMD_SELECT_DESELECT_CARD);
}

/*!
    \brief      获取卡状态
*/
sd_error_typedef_enum sd_cardstatus_get(uint32_t *pcardstatus)
{
    sd_error_typedef_enum status;

    if(NULL == pcardstatus) {
        return SD_PARAMETER_INVALID;
    }

    bsp_sdio_send_cmd(SD_CMD_SEND_STATUS, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SEND_STATUS);
    if(SD_OK != status) {
        return status;
    }
    *pcardstatus = bsp_sdio_get_response(0);
    return status;
}

/*!
    \brief      获取 SD Status
*/
sd_error_typedef_enum sd_sdstatus_get(uint32_t *psdstatus)
{
    sd_error_typedef_enum status;
    uint32_t count = 0;

    if(bsp_sdio_get_response(0) & SD_CARDSTATE_LOCKED) {
        return SD_LOCK_UNLOCK_FAILED;
    }

    bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, 64, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SET_BLOCKLEN);
    if(SD_OK != status) {
        return status;
    }

    bsp_sdio_send_cmd(SD_CMD_APP_CMD, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_APP_CMD);
    if(SD_OK != status) {
        return status;
    }

    sdio_data_config(SD_DATATIMEOUT, 64, SDIO_DATABLOCKSIZE_64BYTES);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOSDIO);
    sdio_dsm_enable();

    bsp_sdio_send_cmd(SD_APPCMD_SD_STATUS, 0, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_APPCMD_SD_STATUS);
    if(SD_OK != status) {
        return status;
    }

    while(!sdio_flag_get(SDIO_FLAG_DTCRCERR | SDIO_FLAG_DTTMOUT |
                         SDIO_FLAG_RXORE | SDIO_FLAG_DTBLKEND | SDIO_FLAG_STBITE)) {
        if(RESET != sdio_flag_get(SDIO_FLAG_RFH)) {
            for(count = 0; count < SD_FIFOHALF_WORDS; count++) {
                *(psdstatus + count) = bsp_sdio_read_word();
            }
            psdstatus += SD_FIFOHALF_WORDS;
        }
    }

    if(RESET != sdio_flag_get(SDIO_FLAG_DTCRCERR)) {
        sdio_flag_clear(SDIO_FLAG_DTCRCERR);
        return SD_DATA_CRC_ERROR;
    } else if(RESET != sdio_flag_get(SDIO_FLAG_DTTMOUT)) {
        sdio_flag_clear(SDIO_FLAG_DTTMOUT);
        return SD_DATA_TIMEOUT;
    } else if(RESET != sdio_flag_get(SDIO_FLAG_RXORE)) {
        sdio_flag_clear(SDIO_FLAG_RXORE);
        return SD_RX_OVERRUN_ERROR;
    } else if(RESET != sdio_flag_get(SDIO_FLAG_STBITE)) {
        sdio_flag_clear(SDIO_FLAG_STBITE);
        return SD_START_BIT_ERROR;
    }
    while(RESET != sdio_flag_get(SDIO_FLAG_RXDTVAL)) {
        *psdstatus = bsp_sdio_read_word();
        ++psdstatus;
    }

    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    psdstatus -= 16;
    for(count = 0; count < 16; count++) {
        psdstatus[count] = ((psdstatus[count] & SD_MASK_0_7BITS)   << 24) |
                           ((psdstatus[count] & SD_MASK_8_15BITS)  <<  8) |
                           ((psdstatus[count] & SD_MASK_16_23BITS) >>  8) |
                           ((psdstatus[count] & SD_MASK_24_31BITS) >> 24);
    }
    return status;
}

/*!
    \brief      停止数据传输
*/
sd_error_typedef_enum sd_transfer_stop(void)
{
    bsp_sdio_send_cmd(SD_CMD_STOP_TRANSMISSION, 0, BSP_SDIO_RESP_SHORT);
    return r1_error_check(SD_CMD_STOP_TRANSMISSION);
}

/*!
    \brief      锁/解锁卡
*/
sd_error_typedef_enum sd_lock_unlock(uint8_t lockstate)
{
    sd_error_typedef_enum status;
    uint8_t cardstate = 0, tempbyte = 0;
    uint32_t pwd1 = 0, pwd2 = 0, response = 0;
    __IO uint32_t timeout = 0;
    uint16_t tempccc = 0;

    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_24_31BITS) >> 24);
    tempccc = (uint16_t)((uint16_t)tempbyte << 4);
    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_16_23BITS) >> 16);
    tempccc |= (uint16_t)((uint16_t)(tempbyte & 0xF0) >> 4);

    if(0 == (tempccc & SD_CCC_LOCK_CARD)) {
        return SD_FUNCTION_UNSUPPORTED;
    }

    pwd1 = (0x01020600 | lockstate);
    pwd2 = 0x03040506;

    sdio_data_config(0, 0, SDIO_DATABLOCKSIZE_1BYTE);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_disable();
    sdio_dma_disable();

    bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, 8, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SET_BLOCKLEN);
    if(SD_OK != status) {
        return status;
    }

    bsp_sdio_send_cmd(SD_CMD_SEND_STATUS, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SEND_STATUS);
    if(SD_OK != status) {
        return status;
    }

    response = bsp_sdio_get_response(0);
    timeout = 100000;
    while((0 == (response & SD_R1_READY_FOR_DATA)) && (timeout > 0)) {
        --timeout;
        bsp_sdio_send_cmd(SD_CMD_SEND_STATUS, (uint32_t)sd_rca << SD_RCA_SHIFT, BSP_SDIO_RESP_SHORT);
        status = r1_error_check(SD_CMD_SEND_STATUS);
        if(SD_OK != status) {
            return status;
        }
        response = bsp_sdio_get_response(0);
    }
    if(0 == timeout) {
        return SD_ERROR;
    }

    bsp_sdio_send_cmd(SD_CMD_LOCK_UNLOCK, 0, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_LOCK_UNLOCK);
    if(SD_OK != status) {
        return status;
    }

    sdio_data_config(SD_DATATIMEOUT, 8, SDIO_DATABLOCKSIZE_8BYTES);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOCARD);
    sdio_dsm_enable();

    bsp_sdio_write_word(pwd1);
    bsp_sdio_write_word(pwd2);

    if(RESET != sdio_flag_get(SDIO_FLAG_DTCRCERR)) {
        sdio_flag_clear(SDIO_FLAG_DTCRCERR);
        return SD_DATA_CRC_ERROR;
    } else if(RESET != sdio_flag_get(SDIO_FLAG_DTTMOUT)) {
        sdio_flag_clear(SDIO_FLAG_DTTMOUT);
        return SD_DATA_TIMEOUT;
    } else if(RESET != sdio_flag_get(SDIO_FLAG_TXURE)) {
        sdio_flag_clear(SDIO_FLAG_TXURE);
        return SD_TX_UNDERRUN_ERROR;
    } else if(RESET != sdio_flag_get(SDIO_FLAG_STBITE)) {
        sdio_flag_clear(SDIO_FLAG_STBITE);
        return SD_START_BIT_ERROR;
    }

    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    status = sd_card_state_get(&cardstate);
    while((SD_OK == status) &&
          ((SD_CARDSTATE_PROGRAMMING == cardstate) || (SD_CARDSTATE_RECEIVING == cardstate))) {
        status = sd_card_state_get(&cardstate);
    }
    return status;
}

/*!
    \brief      获取传输状态
*/
sd_transfer_state_typedef_enum sd_transfer_state_get(void)
{
    if(RESET != sdio_flag_get(SDIO_FLAG_TXRUN | SDIO_FLAG_RXRUN)) {
        return SD_TRANSFER_IN_PROGRESS;
    }
    return SD_NO_TRANSFER;
}

/*!
    \brief      获取卡容量（KB）
*/
uint32_t sd_card_capacity_get(void)
{
    uint8_t tempbyte = 0, devicesize_mult = 0, readblklen = 0;
    uint32_t capacity = 0, devicesize = 0;

    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
       (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype)) {
        tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_8_15BITS) >> 8);
        devicesize |= (uint32_t)((uint32_t)(tempbyte & 0x03) << 10);
        tempbyte = (uint8_t)(sd_csd[1] & SD_MASK_0_7BITS);
        devicesize |= (uint32_t)((uint32_t)tempbyte << 2);
        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_24_31BITS) >> 24);
        devicesize |= (uint32_t)((uint32_t)(tempbyte & 0xC0) >> 6);

        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_16_23BITS) >> 16);
        devicesize_mult = (tempbyte & 0x03) << 1;
        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_8_15BITS) >> 8);
        devicesize_mult |= (tempbyte & 0x80) >> 7;

        tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_16_23BITS) >> 16);
        readblklen = tempbyte & 0x0F;

        capacity = (devicesize + 1) * (1 << (devicesize_mult + 2));
        capacity *= (1 << readblklen);
        capacity /= 1024;
    } else if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        tempbyte = (uint8_t)(sd_csd[1] & SD_MASK_0_7BITS);
        devicesize = (uint32_t)((uint32_t)(tempbyte & 0x3F) << 16);
        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_24_31BITS) >> 24);
        devicesize |= (uint32_t)((uint32_t)tempbyte << 8);
        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_16_23BITS) >> 16);
        devicesize |= (uint32_t)tempbyte;
        capacity = (devicesize + 1) * 512;
    }
    return capacity;
}

/*!
    \brief      解析 CID/CSD 到结构体
*/
sd_error_typedef_enum sd_card_information_get(sd_card_info_struct *pcardinfo)
{
    uint8_t tempbyte = 0;

    if(NULL == pcardinfo) {
        return SD_PARAMETER_INVALID;
    }

    pcardinfo->card_type = cardtype;
    pcardinfo->card_rca  = sd_rca;

    /* ---- CID ---- */
    tempbyte = (uint8_t)((sd_cid[0] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_cid.mid = tempbyte;

    tempbyte = (uint8_t)((sd_cid[0] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_cid.oid = (uint16_t)((uint16_t)tempbyte << 8);
    tempbyte = (uint8_t)((sd_cid[0] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_cid.oid |= (uint16_t)tempbyte;

    tempbyte = (uint8_t)(sd_cid[0] & SD_MASK_0_7BITS);
    pcardinfo->card_cid.pnm0 = (uint32_t)((uint32_t)tempbyte << 24);
    tempbyte = (uint8_t)((sd_cid[1] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_cid.pnm0 |= (uint32_t)((uint32_t)tempbyte << 16);
    tempbyte = (uint8_t)((sd_cid[1] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_cid.pnm0 |= (uint32_t)((uint32_t)tempbyte << 8);
    tempbyte = (uint8_t)((sd_cid[1] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_cid.pnm0 |= (uint32_t)tempbyte;
    tempbyte = (uint8_t)(sd_cid[1] & SD_MASK_0_7BITS);
    pcardinfo->card_cid.pnm1 = tempbyte;

    tempbyte = (uint8_t)((sd_cid[2] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_cid.prv = tempbyte;

    tempbyte = (uint8_t)((sd_cid[2] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_cid.psn = (uint32_t)((uint32_t)tempbyte << 24);
    tempbyte = (uint8_t)((sd_cid[2] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_cid.psn |= (uint32_t)((uint32_t)tempbyte << 16);
    tempbyte = (uint8_t)(sd_cid[2] & SD_MASK_0_7BITS);
    pcardinfo->card_cid.psn |= (uint32_t)tempbyte;
    tempbyte = (uint8_t)((sd_cid[3] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_cid.psn |= (uint32_t)tempbyte;

    tempbyte = (uint8_t)((sd_cid[3] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_cid.mdt = (uint16_t)((uint16_t)(tempbyte & 0x0F) << 8);
    tempbyte = (uint8_t)((sd_cid[3] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_cid.mdt |= (uint16_t)tempbyte;

    tempbyte = (uint8_t)(sd_cid[3] & SD_MASK_0_7BITS);
    pcardinfo->card_cid.cid_crc = (tempbyte & 0xFE) >> 1;

    /* ---- CSD ---- */
    tempbyte = (uint8_t)((sd_csd[0] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_csd.csd_struct = (tempbyte & 0xC0) >> 6;
    tempbyte = (uint8_t)((sd_csd[0] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_csd.taac = tempbyte;
    tempbyte = (uint8_t)((sd_csd[0] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_csd.nsac = tempbyte;
    tempbyte = (uint8_t)(sd_csd[0] & SD_MASK_0_7BITS);
    pcardinfo->card_csd.tran_speed = tempbyte;

    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_csd.ccc = (uint16_t)((uint16_t)tempbyte << 4);
    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_csd.ccc |= (uint16_t)((uint16_t)(tempbyte & 0xF0) >> 4);
    pcardinfo->card_csd.read_bl_len = tempbyte & 0x0F;

    tempbyte = (uint8_t)((sd_csd[1] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_csd.read_bl_partial    = (tempbyte & 0x80) >> 7;
    pcardinfo->card_csd.write_blk_misalign = (tempbyte & 0x40) >> 6;
    pcardinfo->card_csd.read_blk_misalign  = (tempbyte & 0x20) >> 5;
    pcardinfo->card_csd.dsp_imp            = (tempbyte & 0x10) >> 4;

    if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == cardtype) ||
       (SDIO_STD_CAPACITY_SD_CARD_V2_0 == cardtype)) {
        pcardinfo->card_csd.c_size = (uint32_t)((uint32_t)(tempbyte & 0x03) << 10);

        tempbyte = (uint8_t)(sd_csd[1] & SD_MASK_0_7BITS);
        pcardinfo->card_csd.c_size |= (uint32_t)((uint32_t)tempbyte << 2);

        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_24_31BITS) >> 24);
        pcardinfo->card_csd.c_size |= (uint32_t)((uint32_t)(tempbyte & 0xC0) >> 6);
        pcardinfo->card_csd.vdd_r_curr_min = (tempbyte & 0x38) >> 3;
        pcardinfo->card_csd.vdd_r_curr_max = tempbyte & 0x07;

        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_16_23BITS) >> 16);
        pcardinfo->card_csd.vdd_w_curr_min = (tempbyte & 0xE0) >> 5;
        pcardinfo->card_csd.vdd_w_curr_max = (tempbyte & 0x1C) >> 2;
        pcardinfo->card_csd.c_size_mult = (tempbyte & 0x03) << 1;

        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_8_15BITS) >> 8);
        pcardinfo->card_csd.c_size_mult |= (tempbyte & 0x80) >> 7;

        pcardinfo->card_blocksize = 1 << (pcardinfo->card_csd.read_bl_len);
        pcardinfo->card_capacity  = pcardinfo->card_csd.c_size + 1;
        pcardinfo->card_capacity *= (1 << (pcardinfo->card_csd.c_size_mult + 2));
        pcardinfo->card_capacity *= pcardinfo->card_blocksize;
    } else if(SDIO_HIGH_CAPACITY_SD_CARD == cardtype) {
        tempbyte = (uint8_t)(sd_csd[1] & SD_MASK_0_7BITS);
        pcardinfo->card_csd.c_size = (uint32_t)((uint32_t)(tempbyte & 0x3F) << 16);

        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_24_31BITS) >> 24);
        pcardinfo->card_csd.c_size |= (uint32_t)((uint32_t)tempbyte << 8);

        tempbyte = (uint8_t)((sd_csd[2] & SD_MASK_16_23BITS) >> 16);
        pcardinfo->card_csd.c_size |= (uint32_t)tempbyte;

        pcardinfo->card_blocksize = 512;
        pcardinfo->card_capacity  = (pcardinfo->card_csd.c_size + 1) * 512 * 1024;
    }

    pcardinfo->card_csd.erase_blk_en  = (tempbyte & 0x40) >> 6;
    pcardinfo->card_csd.sector_size   = (tempbyte & 0x3F) << 1;

    tempbyte = (uint8_t)(sd_csd[2] & SD_MASK_0_7BITS);
    pcardinfo->card_csd.sector_size |= (tempbyte & 0x80) >> 7;
    pcardinfo->card_csd.wp_grp_size  = (tempbyte & 0x7F);

    tempbyte = (uint8_t)((sd_csd[3] & SD_MASK_24_31BITS) >> 24);
    pcardinfo->card_csd.wp_grp_enable = (tempbyte & 0x80) >> 7;
    pcardinfo->card_csd.r2w_factor    = (tempbyte & 0x1C) >> 2;
    pcardinfo->card_csd.write_bl_len  = (tempbyte & 0x03) << 2;

    tempbyte = (uint8_t)((sd_csd[3] & SD_MASK_16_23BITS) >> 16);
    pcardinfo->card_csd.write_bl_len |= (tempbyte & 0xC0) >> 6;
    pcardinfo->card_csd.write_bl_partial = (tempbyte & 0x20) >> 5;

    tempbyte = (uint8_t)((sd_csd[3] & SD_MASK_8_15BITS) >> 8);
    pcardinfo->card_csd.file_format_grp    = (tempbyte & 0x80) >> 7;
    pcardinfo->card_csd.copy_flag          = (tempbyte & 0x40) >> 6;
    pcardinfo->card_csd.perm_write_protect = (tempbyte & 0x20) >> 5;
    pcardinfo->card_csd.tmp_write_protect  = (tempbyte & 0x10) >> 4;
    pcardinfo->card_csd.file_format        = (tempbyte & 0x0C) >> 2;

    tempbyte = (uint8_t)(sd_csd[3] & SD_MASK_0_7BITS);
    pcardinfo->card_csd.csd_crc = (tempbyte & 0xFE) >> 1;

    return SD_OK;
}

/* ================================================================
 *                      静态辅助函数
 * ================================================================ */

static sd_error_typedef_enum cmdsent_error_check(void)
{
    __IO uint32_t timeout = 100000;

    while((RESET == sdio_flag_get(SDIO_FLAG_CMDSEND)) && (timeout > 0)) {
        --timeout;
    }
    if(0 == timeout) {
        return SD_CMD_RESP_TIMEOUT;
    }
    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    return SD_OK;
}

static sd_error_typedef_enum r1_error_type_check(uint32_t resp)
{
    if(resp & SD_R1_OUT_OF_RANGE)         return SD_OUT_OF_RANGE;
    if(resp & SD_R1_ADDRESS_ERROR)        return SD_ADDRESS_ERROR;
    if(resp & SD_R1_BLOCK_LEN_ERROR)      return SD_BLOCK_LEN_ERROR;
    if(resp & SD_R1_ERASE_SEQ_ERROR)      return SD_ERASE_SEQ_ERROR;
    if(resp & SD_R1_ERASE_PARAM)          return SD_ERASE_PARAM;
    if(resp & SD_R1_WP_VIOLATION)         return SD_WP_VIOLATION;
    if(resp & SD_R1_LOCK_UNLOCK_FAILED)   return SD_LOCK_UNLOCK_FAILED;
    if(resp & SD_R1_COM_CRC_ERROR)        return SD_COM_CRC_ERROR;
    if(resp & SD_R1_ILLEGAL_COMMAND)      return SD_ILLEGAL_COMMAND;
    if(resp & SD_R1_CARD_ECC_FAILED)      return SD_CARD_ECC_FAILED;
    if(resp & SD_R1_CC_ERROR)             return SD_CC_ERROR;
    if(resp & SD_R1_GENERAL_UNKNOWN_ERROR)return SD_GENERAL_UNKNOWN_ERROR;
    if(resp & SD_R1_CSD_OVERWRITE)        return SD_CSD_OVERWRITE;
    if(resp & SD_R1_WP_ERASE_SKIP)        return SD_WP_ERASE_SKIP;
    if(resp & SD_R1_CARD_ECC_DISABLED)    return SD_CARD_ECC_DISABLED;
    if(resp & SD_R1_ERASE_RESET)          return SD_ERASE_RESET;
    if(resp & SD_R1_AKE_SEQ_ERROR)        return SD_AKE_SEQ_ERROR;
    return SD_ERROR;
}

static sd_error_typedef_enum r1_error_check(uint8_t cmdindex)
{
    uint32_t reg_status = 0, resp_r1 = 0;

    reg_status = SDIO_STAT;
    while(!(reg_status & (SDIO_FLAG_CCRCERR | SDIO_FLAG_CMDTMOUT | SDIO_FLAG_CMDRECV))) {
        reg_status = SDIO_STAT;
    }
    if(reg_status & SDIO_FLAG_CCRCERR) {
        sdio_flag_clear(SDIO_FLAG_CCRCERR);
        return SD_CMD_CRC_ERROR;
    } else if(reg_status & SDIO_FLAG_CMDTMOUT) {
        sdio_flag_clear(SDIO_FLAG_CMDTMOUT);
        return SD_CMD_RESP_TIMEOUT;
    }

    if(bsp_sdio_get_resp_cmd_index() != cmdindex) {
        return SD_ILLEGAL_COMMAND;
    }

    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    resp_r1 = bsp_sdio_get_response(0);

    if(SD_ALLZERO == (resp_r1 & SD_R1_ERROR_BITS)) {
        return SD_OK;
    }
    return r1_error_type_check(resp_r1);
}

static sd_error_typedef_enum r2_error_check(void)
{
    uint32_t reg_status = 0;

    reg_status = SDIO_STAT;
    while(!(reg_status & (SDIO_FLAG_CCRCERR | SDIO_FLAG_CMDTMOUT | SDIO_FLAG_CMDRECV))) {
        reg_status = SDIO_STAT;
    }
    if(reg_status & SDIO_FLAG_CCRCERR) {
        sdio_flag_clear(SDIO_FLAG_CCRCERR);
        return SD_CMD_CRC_ERROR;
    } else if(reg_status & SDIO_FLAG_CMDTMOUT) {
        sdio_flag_clear(SDIO_FLAG_CMDTMOUT);
        return SD_CMD_RESP_TIMEOUT;
    }
    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    return SD_OK;
}

static sd_error_typedef_enum r3_error_check(void)
{
    uint32_t reg_status = 0;

    reg_status = SDIO_STAT;
    while(!(reg_status & (SDIO_FLAG_CCRCERR | SDIO_FLAG_CMDTMOUT | SDIO_FLAG_CMDRECV))) {
        reg_status = SDIO_STAT;
    }
    if(reg_status & SDIO_FLAG_CCRCERR) {
        sdio_flag_clear(SDIO_FLAG_CCRCERR);
        return SD_CMD_CRC_ERROR;
    } else if(reg_status & SDIO_FLAG_CMDTMOUT) {
        sdio_flag_clear(SDIO_FLAG_CMDTMOUT);
        return SD_CMD_RESP_TIMEOUT;
    }
    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    return SD_OK;
}

static sd_error_typedef_enum r6_error_check(uint8_t cmdindex, uint16_t *prca)
{
    uint32_t reg_status = 0, response = 0;

    reg_status = SDIO_STAT;
    while(!(reg_status & (SDIO_FLAG_CCRCERR | SDIO_FLAG_CMDTMOUT | SDIO_FLAG_CMDRECV))) {
        reg_status = SDIO_STAT;
    }
    if(reg_status & SDIO_FLAG_CCRCERR) {
        sdio_flag_clear(SDIO_FLAG_CCRCERR);
        return SD_CMD_CRC_ERROR;
    } else if(reg_status & SDIO_FLAG_CMDTMOUT) {
        sdio_flag_clear(SDIO_FLAG_CMDTMOUT);
        return SD_CMD_RESP_TIMEOUT;
    }

    if(bsp_sdio_get_resp_cmd_index() != cmdindex) {
        return SD_ILLEGAL_COMMAND;
    }

    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    response = bsp_sdio_get_response(0);

    if(SD_ALLZERO == (response & (SD_R6_GENERAL_UNKNOWN_ERROR |
                                  SD_R6_ILLEGAL_COMMAND |
                                  SD_R6_COM_CRC_ERROR))) {
        *prca = (uint16_t)(response >> 16);
        return SD_OK;
    }
    return SD_ERROR;
}

static sd_error_typedef_enum r7_error_check(void)
{
    uint32_t reg_status = 0, response = 0;

    reg_status = SDIO_STAT;
    while(!(reg_status & (SDIO_FLAG_CCRCERR | SDIO_FLAG_CMDTMOUT | SDIO_FLAG_CMDRECV))) {
        reg_status = SDIO_STAT;
    }
    if(reg_status & SDIO_FLAG_CCRCERR) {
        sdio_flag_clear(SDIO_FLAG_CCRCERR);
        return SD_CMD_CRC_ERROR;
    } else if(reg_status & SDIO_FLAG_CMDTMOUT) {
        sdio_flag_clear(SDIO_FLAG_CMDTMOUT);
        return SD_CMD_RESP_TIMEOUT;
    }
    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);
    response = bsp_sdio_get_response(0);
    if(SD_CHECK_PATTERN != response) {
        return SD_ERROR;
    }
    return SD_OK;
}

static sd_error_typedef_enum sd_card_state_get(uint8_t *pcardstate)
{
    sd_error_typedef_enum status;
    uint32_t resp_r1;

    bsp_sdio_send_cmd(SD_CMD_SEND_STATUS,
                      (uint32_t)sd_rca << SD_RCA_SHIFT,
                      BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SEND_STATUS);
    if(SD_OK != status) {
        return status;
    }
    resp_r1 = bsp_sdio_get_response(0);
    *pcardstate = (uint8_t)((resp_r1 >> 9) & 0x0F);
    return SD_OK;
}

static sd_error_typedef_enum sd_bus_width_config(uint32_t buswidth)
{
    sd_error_typedef_enum status;

    if((SD_BUS_WIDTH_1BIT != buswidth) && (SD_BUS_WIDTH_4BIT != buswidth)) {
        return SD_PARAMETER_INVALID;
    }

    bsp_sdio_send_cmd(SD_CMD_APP_CMD,
                      (uint32_t)sd_rca << SD_RCA_SHIFT,
                      BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_APP_CMD);
    if(SD_OK != status) {
        return status;
    }

    bsp_sdio_send_cmd(SD_APPCMD_SET_BUS_WIDTH, buswidth, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_APPCMD_SET_BUS_WIDTH);
    if(SD_OK != status) {
        return status;
    }

    sdio_bus_mode_set((SD_BUS_WIDTH_4BIT == buswidth) ?
                      SDIO_BUSMODE_4BIT : SDIO_BUSMODE_1BIT);
    return SD_OK;
}

static sd_error_typedef_enum sd_scr_get(uint16_t rca, uint32_t *pscr)
{
    sd_error_typedef_enum status;
    uint32_t tmp_scr[2] = {0, 0};
    uint32_t i;

    bsp_sdio_send_cmd(SD_CMD_SET_BLOCKLEN, 8, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_SET_BLOCKLEN);
    if(SD_OK != status) {
        return status;
    }

    bsp_sdio_send_cmd(SD_CMD_APP_CMD,
                      (uint32_t)rca << SD_RCA_SHIFT,
                      BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_CMD_APP_CMD);
    if(SD_OK != status) {
        return status;
    }

    sdio_data_config(SD_DATATIMEOUT, 8, SDIO_DATABLOCKSIZE_8BYTES);
    sdio_data_transfer_config(SDIO_TRANSMODE_BLOCK, SDIO_TRANSDIRECTION_TOSDIO);
    sdio_dsm_enable();

    bsp_sdio_send_cmd(SD_APPCMD_SEND_SCR, 0, BSP_SDIO_RESP_SHORT);
    status = r1_error_check(SD_APPCMD_SEND_SCR);
    if(SD_OK != status) {
        return status;
    }

    while(!sdio_flag_get(SDIO_FLAG_DTCRCERR | SDIO_FLAG_DTTMOUT |
                         SDIO_FLAG_RXORE | SDIO_FLAG_DTBLKEND | SDIO_FLAG_STBITE)) {
        if(RESET != sdio_flag_get(SDIO_FLAG_RFH)) {
            for(i = 0; i < 2; i++) {
                tmp_scr[i] = bsp_sdio_read_word();
            }
        }
    }
    sdio_flag_clear(SDIO_MASK_INTC_FLAGS);

    for(i = 0; i < 2; i++) {
        pscr[i] = ((tmp_scr[i] & SD_MASK_0_7BITS)   << 24) |
                  ((tmp_scr[i] & SD_MASK_8_15BITS)  <<  8) |
                  ((tmp_scr[i] & SD_MASK_16_23BITS) >>  8) |
                  ((tmp_scr[i] & SD_MASK_24_31BITS) >> 24);
    }
    return SD_OK;
}

static uint32_t sd_datablocksize_get(uint16_t bytesnumber)
{
    uint8_t blocksize = 0;

    while(bytesnumber) {
        bytesnumber >>= 1;
        blocksize++;
    }
    switch(blocksize) {
    case 1:  return SDIO_DATABLOCKSIZE_1BYTE;
    case 2:  return SDIO_DATABLOCKSIZE_2BYTES;
    case 3:  return SDIO_DATABLOCKSIZE_4BYTES;
    case 4:  return SDIO_DATABLOCKSIZE_8BYTES;
    case 5:  return SDIO_DATABLOCKSIZE_16BYTES;
    case 6:  return SDIO_DATABLOCKSIZE_32BYTES;
    case 7:  return SDIO_DATABLOCKSIZE_64BYTES;
    case 8:  return SDIO_DATABLOCKSIZE_128BYTES;
    case 9:  return SDIO_DATABLOCKSIZE_256BYTES;
    case 10: return SDIO_DATABLOCKSIZE_512BYTES;
    case 11: return SDIO_DATABLOCKSIZE_1024BYTES;
    case 12: return SDIO_DATABLOCKSIZE_2048BYTES;
    default: return SDIO_DATABLOCKSIZE_1BYTE;
    }
}