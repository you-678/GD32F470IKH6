#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
#include "drv_i2s_player.h"

/* =================================================================
 * ?? 全测模式与宏开关
 * ================================================================= */
// 1: 串行跑完所有测试 (死循环被替换为延时)
// 0: 严格按照原厂示例的死循环卡死 (原汁原味)
#define AUTO_RUN_ALL          1   

// 各模块开关
#define RUN_01_LED            1
#define RUN_03_KEY            1
#define RUN_06_USART          1
#define RUN_09_ADC            1
#define RUN_10_DAC            1
#define RUN_12_SPI            1
#define RUN_13_I2S            1
#define RUN_EXMC              1

/* 原汁原味代码的结尾替换宏 */
#if AUTO_RUN_ALL
    #define TEST_END()        delay_1ms(1000) // 全测模式：延时1秒后进入下一个测试
#else
    #define TEST_END()        while(1)         // 原版模式：卡死在死循环
#endif

/* 全局变量提取（原代码中的 static 变量） */
static gd25qxx_dev_t s_flash;
static drv_i2s_player_t s_player;

/* SPI 回调函数 */
static uint8_t xfer(uint8_t t) { return bsp_spi_transfer(BSP_SPI_BUS_FLASH, t); }
static void    csl(void)       { bsp_spi_cs_low(BSP_SPI_BUS_FLASH); }
static void    csh(void)       { bsp_spi_cs_high(BSP_SPI_BUS_FLASH); }
static void    qr(void)        { bsp_spi_quad_enter_read(BSP_SPI_BUS_FLASH); }
static void    qw(void)        { bsp_spi_quad_enter_write(BSP_SPI_BUS_FLASH); }
static void    qx(void)        { bsp_spi_quad_exit(BSP_SPI_BUS_FLASH); }


/* =================================================================
 * 01_GPIO_Running_LED
 * ================================================================= */
#if RUN_01_LED
static void Test_01_LED(void) {
    printf("\r\n========================================\r\n");
    printf("LED Ultimate Test Starts\r\n");
    printf("========================================\r\n");

    printf("[BSP Test 1] bsp_led_init (Init 3 LEDs)\r\n");
    bsp_led_init(LED1); bsp_led_init(LED2); bsp_led_init(LED3); delay_1ms(500);

    printf("[BSP Test 2] bsp_led_on (Turn on one by one)\r\n");
    bsp_led_on(LED1); delay_1ms(300); bsp_led_on(LED2); delay_1ms(300); bsp_led_on(LED3); delay_1ms(500);

    printf("[BSP Test 3] bsp_led_off (Turn off one by one)\r\n");
    bsp_led_off(LED1); delay_1ms(300); bsp_led_off(LED2); delay_1ms(300); bsp_led_off(LED3); delay_1ms(500);

    printf("[BSP Test 4] bsp_led_toggle (Toggle one by one)\r\n");
    bsp_led_toggle(LED1); delay_1ms(300); bsp_led_toggle(LED2); delay_1ms(300); bsp_led_toggle(LED3); delay_1ms(300);
    bsp_led_toggle(LED1); delay_1ms(300); bsp_led_toggle(LED2); delay_1ms(300); bsp_led_toggle(LED3); delay_1ms(300);
    delay_1ms(500);

    printf("[APP Test 1] app_led_all_on (All On)\r\n");
    app_led_all_on(); delay_1ms(1000);

    printf("[APP Test 2] app_led_all_toggle (All Toggle)\r\n");
    app_led_all_toggle(); delay_1ms(1000);

    printf("[APP Test 3] app_led_all_off (All Off)\r\n");
    app_led_all_on(); delay_1ms(500); app_led_all_off(); delay_1ms(1000);

    printf("[APP Test 4] app_led_flash (Flash 3 times)\r\n");
    app_led_flash(3); delay_1ms(500);

    printf("[APP Test 5] app_led_flow_task (Run flow once)\r\n");
    app_led_flow_task(); delay_1ms(1000);

    printf("========================================\r\n");
    printf("All LED functions tested OK! Enter while(1).\r\n");
    printf("========================================\r\n");
    TEST_END();
}
#endif


/* =================================================================
 * 03_EXTI_Key_Interrupt_mode
 * ================================================================= */
#if RUN_03_KEY
static void Test_03_KEY(void) {
    bsp_led_init(LED1); bsp_led_init(LED2); bsp_led_init(LED3);

    printf("\r\n========================================\r\n");
    printf("STAGE 1: ALL POLLING MODE (15 seconds)\r\n");
    printf("Press PA0, PC13, PB14 to test Polling.\r\n");
    printf("========================================\r\n");

    app_key_init_polling();
    for (uint32_t i = 0; i < 15000; i++) {
        app_key_polling_task();
        delay_1ms(1);
    }

    printf("\r\n========================================\r\n");
    printf("STAGE 2: ALL EXTI MODE (Infinite loop)\r\n");
    printf("Press PA0, PC13, PB14 to test EXTI.\r\n");
    printf("========================================\r\n");

    app_key_init_exti();
    TEST_END();
}
#endif


/* =================================================================
 * 06_USART_DMA
 * ================================================================= */
#if RUN_06_USART
static void Test_06_USART(void) {
    uint8_t rx_buf[256];
    uint16_t rx_len = 0;

    bsp_led_init(LED1);
    printf("\r\n========================================\r\n");
    printf("STAGE 1: Printf Test Started (5 seconds)\r\n");
    printf("You should see heartbeats below.\r\n");
    printf("========================================\r\n");

    for (int i = 0; i < 50; i++) {
        printf("[Printf] Heartbeat %d\r\n", i);
        delay_1ms(100);
    }

    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);
    printf("\r\n========================================\r\n");
    printf("STAGE 2: IRQ RX & Printf Echo (10 seconds)\r\n");
    printf("Please send some data to test IRQ.\r\n");
    printf("========================================\r\n");

    for (int i = 0; i < 1000; i++) {
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0';
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }

    printf("\r\n========================================\r\n");
    printf("STAGE 3: IRQ RX & DMA Echo (Infinite loop)\r\n");
    printf("Please send some data to test DMA echo.\r\n");
    printf("========================================\r\n");

    /* 如果开启全测，这里不能死循环，需要特殊处理 */
#if AUTO_RUN_ALL
    for(int j=0; j<300; j++) { // 跑3秒
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0';
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            bsp_usart_send_dma(BSP_USART_COM0, rx_buf, rx_len);
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }
    usart_disable(BSP_USART_COM0);
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
#else
    while(1) {
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0';
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            bsp_usart_send_dma(BSP_USART_COM0, rx_buf, rx_len);
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }
#endif
}
#endif


/* =================================================================
 * 09_ADC0_ADC1_Routine_Parallel_mode
 * ================================================================= */
#if RUN_09_ADC
static void Test_09_ADC(void) {
    bsp_adc_init(BSP_ADC_MODE_INTERNAL);
    printf("\r\n========================================\r\n");
    printf("STAGE 1: INTERNAL ADC (10s)\r\n");
    printf("========================================\r\n");
    for (int i = 0; i < 1000; i++) {
        app_adc_task_internal();
        delay_1ms(10);
    }

    bsp_adc_init(BSP_ADC_MODE_DUAL_DMA_FOLLOW_UP);
    printf("\r\n========================================\r\n");
    printf("STAGE 2: DUAL ADC FOLLOW UP MODE (10s)\r\n");
    printf("========================================\r\n");
    for (int i = 0; i < 1000; i++) {
        app_adc_task_dual_dma();
        delay_1ms(10);
    }

    bsp_adc_init(BSP_ADC_MODE_DUAL_DMA_PARALLEL);
    printf("\r\n========================================\r\n");
    printf("STAGE 3: DUAL ADC PARALLEL MODE (10s)\r\n");
    printf("========================================\r\n");
    for (int i = 0; i < 1000; i++) {
        app_adc_task_dual_dma();
        delay_1ms(10);
    }

    printf("\r\nAll ADC Modes Tested OK!\r\n");
    TEST_END();
}
#endif


/* =================================================================
 * 10_DAC_Output_Voltage_Value
 * ================================================================= */
#if RUN_10_DAC
static void Test_10_DAC(void) {
    printf("\r\n========================================\r\n");
    printf("DAC PA4 Scan Test Started!\r\n");
    printf("Outputting 0V -> 3.3V -> 0V continuously.\r\n");
    printf("Measure PA4 with a multimeter to see the change.\r\n");
    printf("========================================\r\n");

    bsp_dac_init();
#if AUTO_RUN_ALL
    for(int i=0; i<300; i++) { app_dac_scan_task(); delay_1ms(10); } // 跑3秒
#else
    while(1) { app_dac_scan_task(); delay_1ms(10); }
#endif
}
#endif


/* =================================================================
 * 12_SPI_Quad_Flash
 * ================================================================= */
#if RUN_12_SPI
static int flash_test(bsp_spi_mode_t mode) {
    static uint8_t tx[512];
    static uint8_t rx[512];
    const char *name = (mode == BSP_SPI_MODE_HW) ? "HARDWARE" : "SOFTWARE";
    int use_quad;

    printf("\r\n========== %s SPI test ==========\r\n", name);
    bsp_spi_init_mode(BSP_SPI_BUS_FLASH, mode);
    use_quad = bsp_spi_supports_quad(BSP_SPI_BUS_FLASH);

    gd25qxx_bind(&s_flash, xfer, csl, csh, use_quad ? qr : NULL, use_quad ? qw : NULL, use_quad ? qx : NULL);

    if (gd25qxx_probe(&s_flash) != GD25QXX_OK) {
        printf("[%s] probe FAILED, id=0x%06lX\r\n", name, (unsigned long)s_flash.id);
        return -1;
    }
    printf("[%s] id=0x%06lX, cap=%luKB, quad=%s\r\n", name, (unsigned long)s_flash.id, (unsigned long)(s_flash.capacity / 1024), use_quad ? "yes" : "no");

    if (use_quad) gd25qxx_quad_enable(&s_flash);

    for (uint32_t i = 0; i < 512; i++) tx[i] = (uint8_t)i;
    memset(rx, 0, sizeof(rx));

    int rc = gd25qxx_erase_sector(&s_flash, 0x000000);
    if (rc != GD25QXX_OK) return rc;

    rc = gd25qxx_quad_write(&s_flash, tx, 0x000000, 512);
    if (rc != GD25QXX_OK) return rc;

    rc = gd25qxx_quad_read(&s_flash, rx, 0x000000, 512);
    if (rc != GD25QXX_OK) return rc;

    if (memcmp(tx, rx, 512) != 0) {
        printf("[%s] verify FAILED\r\n", name);
        return -2;
    }
    printf("[%s] PASSED\r\n", name);
    return 0;
}

static void Test_12_SPI(void) {
    printf("\r\n===== GD25Qxx dual-mode self test =====\r\n");
    int r_hw = flash_test(BSP_SPI_MODE_HW);
    delay_1ms(10);
    int r_sw = flash_test(BSP_SPI_MODE_SW);

    printf("\r\n===== RESULT =====\r\n");
    printf("  HARDWARE SPI : %s\r\n", r_hw == 0 ? "PASS" : "FAIL");
    printf("  SOFTWARE SPI : %s\r\n", r_sw == 0 ? "PASS" : "FAIL");
    TEST_END();
}
#endif


/* =================================================================
 * 13_I2S_Audio_Player
 * ================================================================= */
#if RUN_13_I2S
static void test_hw(void) {
    printf("\r\n---- HARDWARE I2S ----\r\n");
    bsp_i2s_init_mode(BSP_I2S_BUS_AUDIO, BSP_I2S_MODE_HW);
    int rc = drv_i2s_player_play_blocking(&s_player, wavetestdata, wavetestdata_len);
    if (rc != 0) {
        printf("[HW] FAILED rc=%d\r\n", rc);
        return;
    }
    printf("[HW] rate=%lu ch=%u bits=%u frames=%lu\r\n", (unsigned long)s_player.wav.sample_rate, s_player.wav.num_channels, s_player.wav.bits_per_sample, (unsigned long)s_player.wav.total_frames);
    printf("[HW] PASS\r\n");
}

static void test_sw(void) {
    printf("\r\n---- SOFTWARE I2S ----\r\n");
    bsp_i2s_init_mode(BSP_I2S_BUS_AUDIO, BSP_I2S_MODE_SW);
    int rc = drv_i2s_player_play_blocking(&s_player, wavetestdata, wavetestdata_len);
    if (rc != 0) {
        printf("[SW] FAILED rc=%d\r\n", rc);
        return;
    }
    printf("[SW] PASS\r\n");
}

static void Test_13_I2S(void) {
    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
    nvic_irq_enable(BSP_I2S_AUDIO_IRQn, 0, 1);
    drv_i2s_player_init(&s_player, BSP_I2S_BUS_AUDIO);

    printf("\r\n===== I2S Player Self Test =====\r\n");
    printf("WAV: %lu bytes\r\n", (unsigned long)wavetestdata_len);

    test_hw();
    delay_1ms(500);
    test_sw();

    printf("\r\n===== ALL DONE =====\r\n");
    TEST_END();
}
#endif


/* =================================================================
 * 当前工程默认 EXMC 存储测试
 * ================================================================= */
#if RUN_EXMC
static void Test_EXMC(void) {
    printf("\r\n========================================\r\n");
    printf("GD32F470 Full Test Boot\r\n");
    printf("========================================\r\n");

    printf("\r\n[STAGE 1] POLLING TEST (5 seconds)...\r\n");
    printf("Press PA0/PC13/PB14 to toggle LEDs.\r\n");
    app_key_init_polling();
    for (uint32_t i = 0; i < 5000; i++) {
        app_key_polling_task();
        delay_1ms(1);
    }

    printf("\r\n[STAGE 2] EXMC STORAGE TEST...\r\n");
    for (int m = 0; m < STORAGE_MODE_MAX; m++) {
        drv_storage_run((storage_mode_t)m);
        delay_1ms(1000);
    }

    printf("\r\n========================================\r\n");
    printf("ALL TESTS COMPLETED SUCCESSFULLY!\r\n");
    printf("You can still press keys to toggle LEDs.\r\n");
    printf("========================================\r\n");

    app_key_init_exti(); 
    TEST_END();
}
#endif


/* =================================================================
 * 主函数入口
 * ================================================================= */
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);

#if RUN_01_LED
    Test_01_LED();
#endif
#if RUN_03_KEY
    Test_03_KEY();
#endif
#if RUN_06_USART
    Test_06_USART();
#endif
#if RUN_09_ADC
    Test_09_ADC();
#endif
#if RUN_10_DAC
    Test_10_DAC();
#endif
#if RUN_12_SPI
    Test_12_SPI();
#endif
#if RUN_13_I2S
    Test_13_I2S();
#endif
#if RUN_EXMC
    Test_EXMC();
#endif

    /* 如果所有测试都跑完了，死循环兜底 */
    while(1) { delay_1ms(1000); }
}