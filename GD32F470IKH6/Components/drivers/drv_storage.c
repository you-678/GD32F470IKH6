#include "drv_storage.h"

#define SDRAM_TEST_ADDR     0x0000U
#define SDRAM_TEST_SIZE     0x0400U
#define NAND_TEST_ADDR      0x00000000U
#define NAND_TEST_SIZE      0x0100U

const char *drv_storage_mode_name(storage_mode_t mode) {
    switch (mode) {
    case STORAGE_MODE_SDRAM:       return "SDRAM";
    case STORAGE_MODE_SDRAM_SLEEP: return "SDRAM-SLEEP";
    case STORAGE_MODE_NAND:        return "NAND";
    default:                       return "?";
    }
}

/* ================= 1. 普通 SDRAM 读写 ================= */
static int test_sdram(void) {
    static uint8_t tx[SDRAM_TEST_SIZE];
    static uint8_t rx[SDRAM_TEST_SIZE];

    bsp_exmc_init_mode(BSP_EXMC_MODE_SDRAM);
    for (uint32_t i = 0; i < SDRAM_TEST_SIZE; i++) tx[i] = (uint8_t)i;
    bsp_sdram_write(tx, SDRAM_TEST_ADDR, SDRAM_TEST_SIZE);
    bsp_sdram_read (rx, SDRAM_TEST_ADDR, SDRAM_TEST_SIZE);

    if (memcmp(tx, rx, SDRAM_TEST_SIZE) != 0) return -1;
    return 0;
}

/* ================= 2. SDRAM 深睡 + 唤醒保持 ================= */
static int test_sdram_sleep(void) {
    static uint8_t tx[SDRAM_TEST_SIZE];
    static uint8_t rx[SDRAM_TEST_SIZE];

    bsp_exmc_init_mode(BSP_EXMC_MODE_SDRAM);
    for (uint32_t i = 0; i < SDRAM_TEST_SIZE; i++) tx[i] = (uint8_t)(0x55 + i);
    bsp_sdram_write(tx, SDRAM_TEST_ADDR, SDRAM_TEST_SIZE);

    /* 强行覆盖寄存器配置，摆脱之前 STAGE 1 轮询留下的任何状态 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SYSCFG);
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN0);
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_0);
    nvic_irq_enable(EXTI0_IRQn, 0, 0);

    printf("  Enter MCU deepsleep...\r\n");
    printf("  >>> PRESS PA0 TO WAKEUP <<<\r\n");
    delay_1ms(1000); 

    bsp_led_on(LED1); /* 睡了 */

    /* 核心：强行关闭 SysTick 中断，防止干扰，然后直接 WFI */
    NVIC_ClearPendingIRQ(SysTick_IRQn);
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk; 

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();

    /* ==========================================
     * 唤醒后立刻执行！
     * ========================================== */
    bsp_led_off(LED1); /* 醒了！灯灭 */
    bsp_led_on(LED2);  /* 亮 LED2 */

    /* 重新打开 SysTick */
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk; 

    /* 恢复 200MHz 时钟 */
    bsp_exmc_restore_clock(); 
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    bsp_led_on(LED3); 
    printf("  Woke up! Clock restored.\r\n");

    /* 唤醒后重新初始化 EXMC，彻底规避自刷新状态问题 */
    printf("  Re-init SDRAM...\r\n");
    bsp_exmc_init_mode(BSP_EXMC_MODE_SDRAM);

    /* 校验掉电保持 */
    bsp_sdram_read(rx, SDRAM_TEST_ADDR, SDRAM_TEST_SIZE);
    if (memcmp(tx, rx, SDRAM_TEST_SIZE) != 0) {
        printf("  Data mismatch!\r\n");
        return -1;
    }
    return 0;
}

/* ================= 3. NAND 读写 ================= */
static int test_nand(void) {
    static uint8_t tx[NAND_TEST_SIZE];
    static uint8_t rx[NAND_TEST_SIZE];
    bsp_nand_id_t id;

    bsp_exmc_init_mode(BSP_EXMC_MODE_NAND);
    bsp_nand_reset();
    bsp_nand_read_id(&id);
    printf("  NAND ID: 0x%02X 0x%02X 0x%02X 0x%02X\r\n", id.maker_id, id.device_id, id.third_id, id.fourth_id);

    if (!((id.maker_id == 0xC8 || id.maker_id == 0xAD) && id.device_id == 0xF1)) {
        return -1; /* 如果板子没焊 NAND，这里会报 -1，属于正常现象 */
    }

    for (uint32_t i = 0; i < NAND_TEST_SIZE; i++) tx[i] = (uint8_t)i;
    if (bsp_nand_write(NAND_TEST_ADDR, tx, NAND_TEST_SIZE) != 0) return -2;
    if (bsp_nand_read (NAND_TEST_ADDR, rx, NAND_TEST_SIZE) != 0) return -3;
    if (memcmp(tx, rx, NAND_TEST_SIZE) != 0) return -4;
    return 0;
}

/* ================= 统一入口 ================= */
int drv_storage_run(storage_mode_t mode) {
    int rc = -100;
    printf("\r\n---- [%s] ----\r\n", drv_storage_mode_name(mode));

    switch (mode) {
    case STORAGE_MODE_SDRAM:        rc = test_sdram();       break;
    case STORAGE_MODE_SDRAM_SLEEP:  rc = test_sdram_sleep(); break;
    case STORAGE_MODE_NAND:         rc = test_nand();        break;
    default: break;
    }

    if (rc == 0) printf("[%s] PASS\r\n", drv_storage_mode_name(mode));
    else         printf("[%s] FAIL, rc=%d\r\n", drv_storage_mode_name(mode), rc);

    return rc;
}