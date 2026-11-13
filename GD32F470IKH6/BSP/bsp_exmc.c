#include "bsp_exmc.h"

#define SDRAM_BURST_LEN_1          ((uint16_t)0x0000)
#define SDRAM_BURST_TYPE_SEQ       ((uint16_t)0x0000)
#define SDRAM_CAS_LAT_3            ((uint16_t)0x0030)
#define SDRAM_OP_STANDARD          ((uint16_t)0x0000)
#define SDRAM_WB_SINGLE            ((uint16_t)0x0200)
#define SDRAM_TIMEOUT              ((uint32_t)0x0000FFFF)

#define NAND_CMD_AREA              (*(__IO uint8_t *)(BSP_NAND_BASE | BSP_NAND_CMD_OFS))
#define NAND_ADDR_AREA             (*(__IO uint8_t *)(BSP_NAND_BASE | BSP_NAND_ADDR_OFS))
#define NAND_DATA_AREA             (*(__IO uint8_t *)(BSP_NAND_BASE | BSP_NAND_DATA_OFS))
#define NAND_ROW_ADDR(a)           ((a).page + ((a).block + (a).zone * BSP_NAND_ZONE_SIZE) * BSP_NAND_BLOCK_SIZE)

#define NAND_CMD_READ_1ST          0x00
#define NAND_CMD_READ_2ND          0x30
#define NAND_CMD_WRITE_1ST         0x80
#define NAND_CMD_WRITE_2ND         0x10
#define NAND_CMD_ERASE_1ST         0x60
#define NAND_CMD_ERASE_2ND         0xD0
#define NAND_CMD_READID            0x90
#define NAND_CMD_STATUS            0x70
#define NAND_CMD_RESET             0xFF

#define NAND_STATUS_BUSY           0x00
#define NAND_STATUS_ERROR          0x01
#define NAND_STATUS_READY          0x40
#define NAND_STATUS_TIMEOUT        0x80

typedef struct {
    uint16_t zone;
    uint16_t block;
    uint16_t page;
    uint16_t page_in_offset;
} nand_addr_t;

static bsp_exmc_mode_t s_mode = BSP_EXMC_MODE_MAX;

static void sdram_pins_config(void) {
    rcu_periph_clock_enable(RCU_GPIOC); rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE); rcu_periph_clock_enable(RCU_GPIOF);
    rcu_periph_clock_enable(RCU_GPIOG); rcu_periph_clock_enable(RCU_GPIOH);

    gpio_af_set(GPIOC, GPIO_AF_12, GPIO_PIN_2 | GPIO_PIN_5);
    gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2 | GPIO_PIN_5);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2 | GPIO_PIN_5);

    gpio_af_set(GPIOD, GPIO_AF_12, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15);

    gpio_af_set(GPIOE, GPIO_AF_12, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
    gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    gpio_af_set(GPIOF, GPIO_AF_12, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    gpio_af_set(GPIOG, GPIO_AF_12, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_15);
    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_15);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_15);

    gpio_af_set(GPIOH, GPIO_AF_12, GPIO_PIN_5);
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_5);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
}

static void sdram_init(void) {
    exmc_sdram_parameter_struct         sd;
    exmc_sdram_timing_parameter_struct  tm;
    exmc_sdram_command_parameter_struct cmd;
    uint32_t timeout = SDRAM_TIMEOUT;

    tm.load_mode_register_delay  = 2;
    tm.exit_selfrefresh_delay    = 7;
    tm.row_address_select_delay  = 5;
    tm.auto_refresh_delay        = 6;
    tm.write_recovery_delay      = 2;
    tm.row_precharge_delay       = 2;
    tm.row_to_column_delay       = 2;

    sd.sdram_device              = EXMC_SDRAM_DEVICE0;
    sd.column_address_width      = EXMC_SDRAM_COW_ADDRESS_9;
    sd.row_address_width         = EXMC_SDRAM_ROW_ADDRESS_13;
    sd.data_width                = EXMC_SDRAM_DATABUS_WIDTH_16B;
    sd.internal_bank_number      = EXMC_SDRAM_4_INTER_BANK;
    sd.cas_latency               = EXMC_CAS_LATENCY_3_SDCLK;
    sd.write_protection          = DISABLE;
    sd.sdclock_config            = EXMC_SDCLK_PERIODS_3_HCLK;
    sd.brust_read_switch         = ENABLE; /* 原厂拼写 */
    sd.pipeline_read_delay       = EXMC_PIPELINE_DELAY_2_HCLK;
    sd.timing                    = &tm;
    exmc_sdram_init(&sd);

    cmd.command = EXMC_SDRAM_CLOCK_ENABLE;
    cmd.bank_select = EXMC_SDRAM_DEVICE0_SELECT;
    cmd.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_2_SDCLK;
    cmd.mode_register_content = 0;
    while((exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) timeout--;
    exmc_sdram_command_config(&cmd);

    delay_1ms(10);

    cmd.command = EXMC_SDRAM_PRECHARGE_ALL;
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) timeout--;
    exmc_sdram_command_config(&cmd);

    cmd.command = EXMC_SDRAM_AUTO_REFRESH;
    cmd.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_9_SDCLK;
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) timeout--;
    exmc_sdram_command_config(&cmd);

    uint32_t mode_reg = (uint32_t)(SDRAM_BURST_LEN_1 | SDRAM_BURST_TYPE_SEQ | SDRAM_CAS_LAT_3 | SDRAM_OP_STANDARD | SDRAM_WB_SINGLE);
    cmd.command = EXMC_SDRAM_LOAD_MODE_REGISTER;
    cmd.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_2_SDCLK;
    cmd.mode_register_content = mode_reg;
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) timeout--;
    exmc_sdram_command_config(&cmd);

    exmc_sdram_refresh_count_set(761);
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0)) timeout--;
}

static void nand_pins_config(void) {
    gpio_af_set(GPIOD, GPIO_AF_12, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_7);
    gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_7);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_7);
    gpio_mode_set(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_6);
}

static void nand_init(void) {
    exmc_nand_parameter_struct nand;
    exmc_nand_pccard_timing_parameter_struct tm;

    tm.setuptime = 6; tm.waittime = 5; tm.holdtime = 3; tm.databus_hiztime = 2;

    nand.nand_bank = EXMC_BANK1_NAND;
    nand.ecc_size = EXMC_ECC_SIZE_2048BYTES;
    nand.atr_latency = EXMC_ALE_RE_DELAY_2_HCLK;
    nand.ctr_latency = EXMC_CLE_RE_DELAY_2_HCLK;
    nand.ecc_logic = ENABLE;
    nand.databus_width = EXMC_NAND_DATABUS_WIDTH_8B;
    nand.wait_feature = ENABLE;
    nand.common_space_timing = &tm;
    nand.attribute_space_timing = &tm;

    exmc_nand_init(&nand);
    exmc_nand_enable(EXMC_BANK1_NAND);
}

void bsp_exmc_init_mode(bsp_exmc_mode_t mode) {
    rcu_periph_clock_enable(RCU_EXMC);
    if (mode == BSP_EXMC_MODE_SDRAM) {
        if (s_mode == BSP_EXMC_MODE_NAND) exmc_nand_disable(EXMC_BANK1_NAND);
        sdram_pins_config();
        sdram_init();
    } else if (mode == BSP_EXMC_MODE_NAND) {
        sdram_pins_config();
        nand_pins_config();
        nand_init();
    }
    s_mode = mode;
}

bsp_exmc_mode_t bsp_exmc_get_mode(void) { return s_mode; }

void bsp_sdram_write(uint8_t *buf, uint32_t addr, uint32_t len) {
    uint32_t base = BSP_SDRAM_BASE + addr;
    while (len--) *(__IO uint8_t *)base++ = *buf++;
}

void bsp_sdram_read(uint8_t *buf, uint32_t addr, uint32_t len) {
    uint32_t base = BSP_SDRAM_BASE + addr;
    while (len--) *buf++ = *(__IO uint8_t *)base++;
}

void bsp_sdram_enter_self_refresh(void) {
    exmc_sdram_command_parameter_struct cmd;
    cmd.command = EXMC_SDRAM_SELF_REFRESH;
    cmd.bank_select = EXMC_SDRAM_DEVICE0_SELECT;
    cmd.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_1_SDCLK;
    cmd.mode_register_content = 0;
    while (exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) == SET);
    exmc_sdram_command_config(&cmd);
    while (exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) == SET);
    while (exmc_sdram_bankstatus_get(EXMC_SDRAM_DEVICE0) != EXMC_SDRAM_DEVICE_SELF_REFRESH);
}

void bsp_sdram_exit_self_refresh(void) {
    exmc_sdram_command_parameter_struct cmd;
    cmd.command = EXMC_SDRAM_NORMAL_OPERATION;
    cmd.bank_select = EXMC_SDRAM_DEVICE0;
    cmd.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_2_SDCLK;
    cmd.mode_register_content = 0;
    exmc_sdram_command_config(&cmd);
    while (exmc_flag_get(EXMC_SDRAM_DEVICE0, EXMC_SDRAM_FLAG_NREADY) != RESET);
    while (exmc_sdram_bankstatus_get(EXMC_SDRAM_DEVICE0) != EXMC_SDRAM_DEVICE_NORMAL);
}

static uint8_t nand_status_read(void) {
    NAND_CMD_AREA = NAND_CMD_STATUS;
    uint8_t d = NAND_DATA_AREA;
    if (d & NAND_STATUS_ERROR) return NAND_STATUS_ERROR;
    if (d & NAND_STATUS_READY) return NAND_STATUS_READY;
    return NAND_STATUS_BUSY;
}

static uint8_t nand_wait_ready(void) {
    uint32_t timeout = 0x10000;
    uint8_t st = nand_status_read();
    while ((st != NAND_STATUS_READY) && timeout--) st = nand_status_read();
    return (timeout == 0) ? NAND_STATUS_TIMEOUT : st;
}

static int nand_write_page(uint8_t *buf, nand_addr_t a, uint16_t len) {
    NAND_CMD_AREA  = NAND_CMD_WRITE_1ST;
    NAND_ADDR_AREA = (uint8_t)(a.page_in_offset);
    NAND_ADDR_AREA = (uint8_t)(a.page_in_offset >> 8);
    uint32_t row = NAND_ROW_ADDR(a);
    NAND_ADDR_AREA = (uint8_t)(row);
    NAND_ADDR_AREA = (uint8_t)(row >> 8);
    for (uint16_t i = 0; i < len; i++) NAND_DATA_AREA = buf[i];
    NAND_CMD_AREA = NAND_CMD_WRITE_2ND;
    return (nand_wait_ready() == NAND_STATUS_READY) ? 0 : -1;
}

static int nand_read_page(uint8_t *buf, nand_addr_t a, uint16_t len) {
    NAND_CMD_AREA  = NAND_CMD_READ_1ST;
    NAND_ADDR_AREA = (uint8_t)(a.page_in_offset);
    NAND_ADDR_AREA = (uint8_t)(a.page_in_offset >> 8);
    uint32_t row = NAND_ROW_ADDR(a);
    NAND_ADDR_AREA = (uint8_t)(row);
    NAND_ADDR_AREA = (uint8_t)(row >> 8);
    NAND_CMD_AREA  = NAND_CMD_READ_2ND;
    for (uint16_t i = 0; i < len; i++) buf[i] = NAND_DATA_AREA;
    return (nand_wait_ready() == NAND_STATUS_READY) ? 0 : -1;
}

int bsp_nand_erase_block(uint32_t block) {
    NAND_CMD_AREA  = NAND_CMD_ERASE_1ST;
    uint32_t row = block << 6;
    NAND_ADDR_AREA = (uint8_t)(row);
    NAND_ADDR_AREA = (uint8_t)(row >> 8);
    NAND_CMD_AREA  = NAND_CMD_ERASE_2ND;
    return (nand_wait_ready() == NAND_STATUS_READY) ? 0 : -1;
}

int bsp_nand_reset(void) {
    NAND_CMD_AREA = NAND_CMD_RESET;
    return (nand_wait_ready() == NAND_STATUS_READY) ? 0 : -1;
}

void bsp_nand_read_id(bsp_nand_id_t *id) {
    NAND_CMD_AREA  = NAND_CMD_READID;
    NAND_ADDR_AREA = 0x00;
    uint32_t d = *(__IO uint32_t *)(BSP_NAND_BASE | BSP_NAND_DATA_OFS);
    id->maker_id  = (uint8_t)(d);
    id->device_id = (uint8_t)(d >> 8);
    id->third_id  = (uint8_t)(d >> 16);
    id->fourth_id = (uint8_t)(d >> 24);
}

static void nand_addr_from_mem(uint32_t memaddr, nand_addr_t *a) {
    uint32_t block_bytes = BSP_NAND_BLOCK_SIZE * BSP_NAND_PAGE_SIZE;
    a->zone  = (uint16_t)(memaddr / (block_bytes * BSP_NAND_ZONE_SIZE));
    a->block = (uint16_t)(memaddr / block_bytes);
    uint32_t off = memaddr % block_bytes;
    a->page  = (uint16_t)(off / BSP_NAND_PAGE_SIZE);
    a->page_in_offset = (uint16_t)(off % BSP_NAND_PAGE_SIZE);
}

int bsp_nand_write(uint32_t memaddr, uint8_t *buf, uint16_t len) {
    nand_addr_t a;
    nand_addr_from_mem(memaddr, &a);
    if (bsp_nand_erase_block(a.block) != 0) return -1;
    while (len) {
        uint16_t chunk = BSP_NAND_PAGE_SIZE - a.page_in_offset;
        if (chunk > len) chunk = len;
        if (nand_write_page(buf, a, chunk) != 0) return -1;
        buf += chunk; len -= chunk; a.page_in_offset = 0; a.page++;
        if (a.page >= BSP_NAND_BLOCK_SIZE) { a.page = 0; a.block++; }
    }
    return 0;
}

int bsp_nand_read(uint32_t memaddr, uint8_t *buf, uint16_t len) {
    nand_addr_t a;
    nand_addr_from_mem(memaddr, &a);
    while (len) {
        uint16_t chunk = BSP_NAND_PAGE_SIZE - a.page_in_offset;
        if (chunk > len) chunk = len;
        if (nand_read_page(buf, a, chunk) != 0) return -1;
        buf += chunk; len -= chunk; a.page_in_offset = 0; a.page++;
        if (a.page >= BSP_NAND_BLOCK_SIZE) { a.page = 0; a.block++; }
    }
    return 0;
}

/* 修复：恢复 200MHz 时钟，极其关键的是加了 Flash 等待周期 FMC_WS = 5 */
void bsp_exmc_restore_clock(void) {
    uint32_t timeout = 0;

    /* 1. 开启外部晶振 */
    RCU_CTL |= RCU_CTL_HXTALEN;
    while(((RCU_CTL & RCU_CTL_HXTALSTB) == 0) && (timeout < 0xFFFFFF)) { timeout++; }
    if((RCU_CTL & RCU_CTL_HXTALSTB) == 0) { return; }

    /* 2. 极其关键：设置 Flash 等待周期为 5，否则 200MHz 必死机 */
    FMC_WS = (FMC_WS & (~FMC_WC_WSCNT)) | 5;

    /* 3. 配置分频系数 */
    RCU_CFG0 |= RCU_AHB_CKSYS_DIV1;
    RCU_CFG0 |= RCU_APB2_CKAHB_DIV2;
    RCU_CFG0 |= RCU_APB1_CKAHB_DIV4;

    /* 4. 开启 PMU 高驱动 */
    RCU_APB1EN |= RCU_APB1EN_PMUEN;
    PMU_CTL |= PMU_CTL_LDOVS;

    /* 5. 配置并开启 PLL */
    RCU_PLL = (25U | (480U << 6U) | (((2U >> 1U) - 1U) << 16U) | RCU_PLLSRC_HXTAL | (10U << 24U));
    RCU_CTL |= RCU_CTL_PLLEN;
    timeout = 0;
    while (((RCU_CTL & RCU_CTL_PLLSTB) == 0) && (timeout < 0xFFFFFF)) { timeout++; }

    /* 6. 使能高驱动模式 */
    PMU_CTL |= PMU_CTL_HDEN;
    while ((PMU_CS & PMU_CS_HDRF) == 0);
    PMU_CTL |= PMU_CTL_HDS;
    while ((PMU_CS & PMU_CS_HDSRF) == 0);

    /* 7. 切换主时钟到 PLL */
    RCU_CFG0 &= ~RCU_CFG0_SCS;
    RCU_CFG0 |= RCU_CKSYSSRC_PLLP;
    while ((RCU_CFG0 & RCU_SCSS_PLLP) == 0);
}