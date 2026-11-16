#include "app_sdcard.h"

static sd_card_info_struct s_cardinfo;
static uint32_t s_buf_write[512];
static uint32_t s_buf_read[512];

/*!
    \brief      初始化 SD 卡应用层
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_sdcard_init(void)
{
    /* 预留：如需按键/插入检测，在这里初始化 */
}

/*!
    \brief      配置 SD 卡（上电、识别、选卡、总线、传输模式），带 5 次重试
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_sdcard_config(void)
{
    uint16_t retry = 5;
    sd_error_typedef_enum status;

    do {
        status = sd_init();
        if(SD_OK == status) status = sd_card_information_get(&s_cardinfo);
        if(SD_OK == status) status = sd_card_select_deselect(s_cardinfo.card_rca);

#ifdef BUSMODE_4BIT
        if(SD_OK == status) status = sd_bus_mode_config(SDIO_BUSMODE_4BIT);
#else
        if(SD_OK == status) status = sd_bus_mode_config(SDIO_BUSMODE_1BIT);
#endif

#ifdef DMA_MODE
        if(SD_OK == status) status = sd_transfer_mode_config(SD_DMA_MODE);
#else
        if(SD_OK == status) status = sd_transfer_mode_config(SD_POLLING_MODE);
#endif
    } while((SD_OK != status) && (--retry));

    if(!retry) {
        printf("\r\n[SD] Card init failed!\r\n");
        bsp_led_on(LED1);
        bsp_led_on(LED3);
        bsp_led_off(LED2);
        return;
    }
    printf("\r\n[SD] Card init success, capacity=%uKB\r\n", sd_card_capacity_get());
}

/*!
    \brief      打印 SD 卡信息
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_sdcard_info_print(void)
{
    uint8_t sd_spec, sd_spec3, sd_spec4, sd_security;
    uint16_t temp_ccc;

    printf("\r\n[SD] Card information:");

    sd_spec  = (sd_scr[1] & 0x0F000000) >> 24;
    sd_spec3 = (sd_scr[1] & 0x00008000) >> 15;
    sd_spec4 = (sd_scr[1] & 0x00000400) >> 10;
    if(2 == sd_spec) {
        if(1 == sd_spec3) {
            if(1 == sd_spec4) {
                printf("\r\n## Card version 4.xx ##");
            } else {
                printf("\r\n## Card version 3.0x ##");
            }
        } else {
            printf("\r\n## Card version 2.00 ##");
        }
    } else if(1 == sd_spec) {
        printf("\r\n## Card version 1.10 ##");
    } else if(0 == sd_spec) {
        printf("\r\n## Card version 1.0x ##");
    }

    sd_security = (sd_scr[1] & 0x00700000) >> 20;
    if(2 == sd_security) {
        printf("\r\n## SDSC card ##");
    } else if(3 == sd_security) {
        printf("\r\n## SDHC card ##");
    } else if(4 == sd_security) {
        printf("\r\n## SDXC card ##");
    }

    printf("\r\n## Device size is %uKB ##", sd_card_capacity_get());
    printf("\r\n## Block size is 512B ##");
    printf("\r\n## Block count is %u ##", (s_cardinfo.card_csd.c_size + 1) * 1024);

    temp_ccc = s_cardinfo.card_csd.ccc;
    printf("\r\n## CardCommandClasses is: %x ##", temp_ccc);
    if((SD_CCC_BLOCK_READ & temp_ccc) && (SD_CCC_BLOCK_WRITE & temp_ccc)) {
        printf("\r\n## Block operation supported ##");
    }
    if(SD_CCC_ERASE & temp_ccc) {
        printf("\r\n## Erase supported ##");
    }
    if(SD_CCC_LOCK_CARD & temp_ccc) {
        printf("\r\n## Lock unlock supported ##");
    }
}

/*!
    \brief      SD 卡读写自检（单块 + 多块）
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_sdcard_test(void)
{
    uint16_t i;
    sd_error_typedef_enum status;

    for(i = 0; i < 512; i++) {
        s_buf_write[i] = (uint32_t)i;
    }

    /* ---- 单块写 ---- */
    status = sd_block_write(s_buf_write, 100U * 512U, 512U);
    if(SD_OK != status) {
        printf("\r\n[SD] Block write fail! code=%d", status);
        bsp_led_on(LED1); bsp_led_on(LED3); bsp_led_off(LED2);
        return;
    }
    printf("\r\n[SD] Block write success!");

    /* ---- 单块读 ---- */
    status = sd_block_read(s_buf_read, 100U * 512U, 512U);
    if(SD_OK != status) {
        printf("\r\n[SD] Block read fail! code=%d", status);
        bsp_led_on(LED1); bsp_led_on(LED3); bsp_led_off(LED2);
        return;
    }
    printf("\r\n[SD] Block read success!");

    /* ---- 多块写 ---- */
    status = sd_multiblocks_write(s_buf_write, 200U * 512U, 512U, 3U);
    if(SD_OK != status) {
        printf("\r\n[SD] Multi write fail! code=%d", status);
        bsp_led_on(LED1); bsp_led_on(LED3); bsp_led_off(LED2);
        return;
    }
    printf("\r\n[SD] Multi write success!");

    /* ---- 多块读 ---- */
    status = sd_multiblocks_read(s_buf_read, 200U * 512U, 512U, 3U);
    if(SD_OK != status) {
        printf("\r\n[SD] Multi read fail! code=%d", status);
        bsp_led_on(LED1); bsp_led_on(LED3); bsp_led_off(LED2);
        return;
    }
    printf("\r\n[SD] Multi read success!");

    /* ---- 全部通过 ---- */
    bsp_led_on(LED1); bsp_led_on(LED2); bsp_led_on(LED3);
    printf("\r\n[SD] All tests passed!\r\n");
}