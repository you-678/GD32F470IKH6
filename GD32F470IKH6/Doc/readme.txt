================================================================
GD32F470IKH6 项目文档 (Doc/readme.txt)
================================================================
说明：本文档汇总项目的「分层架构」与「官方示例例程（29 个）」。
其中架构树与 main 调用函数部分条理清晰地编排，作为历史与参考。

----------------------------------------------------------------
一、项目架构（Project Architecture）
----------------------------------------------------------------
Project_Root/
 │
 ├── 0_Firmware/               // [底层] 原厂库 (绝对不改)
 │    ├── CMSIS/               // 包含 system_gd32f4xx.c 和启动文件
 │    └── StdPeriph/           // 包含 gd32f4xx_gpio.c 等外设库
 │
 ├── 1_BSP/                    // [驱动层] 板级支持包（直接碰寄存器）
 │    ├── inc/
 │    │    ├── bsp_pin_def.h   //  全局硬件地图（引脚、时钟、外设编号）
 │    │    ├── bsp_led.h
 │    │    ├── bsp_key.h
 │    │    ├── bsp_usart.h
 │    │    ├── bsp_i2c.h       // (包含硬/软 I2C)
 │    │    ├── bsp_spi.h       // (硬件/软件 SPI，Flash/屏幕)
 │    │    ├── bsp_i2s.h       // (硬件/软件 I2S)
 │    │    ├── bsp_exmc.h      // (外部存储 SDRAM/NAND)
 │    │    └── bsp_adc.h / bsp_dac.h
 │    └── src/
 │         └── (对应的 .c 文件)
 │
 ├── 2_OSAL/                   // [系统抽象] 操作系统适配层（预留空目录）
 ├── 3_Components/             // [组件层] 核心算法、协议解析、第三方设备驱动
 │    ├── drivers/             //  传感器/外设驱动 (芯片无关)
 │    │    ├── drv_at24c02.c/h  // EEPROM
 │    │    ├── drv_storage.c/h  // 存储统一调度（SDRAM/NAND）
 │    │    ├── drv_gd25qxx.c/h  // SPI Flash
 │    │    ├── drv_wav.c/h      // WAV 解析
 │    │    ├── drv_i2s_player.c/h // I2S 播放器
 │    │    └── wave_data.c/h    // 嵌入的测试 WAV 数据
 │    ├── protocols/           //  通信协议（预留空目录）
 │    └── third_party/         //  第三方开源库（预留空目录）
 │
 ├── 4_Services/               // [服务层] 系统级业务管理中心（预留空目录）
 ├── 5_App/                    // [应用层] 纯业务逻辑 (完全不碰硬件)
 │    └── app_led/key/usart/adc/dac/i2c_eeprom (.c/.h)
 ├── 6_Core/                   // [核心层] 启动与中断
 │    ├── main.c               // 默认例程：GD32F470 Full Test
 │    ├── gd32f4xx_it.c        // 所有中断服务函数入口
 │    └── systick.c            // 系统心跳
 ├── 7_Project/                // Keil 工程文件存放处 (.uvprojx 等)
 └── 8_Doc/                    // 文档 (README, 硬件引脚表, 通信协议手册)

注：以上为「规划中的分层目录」示意（编号 0~8 对应 BSP/App/Components 等
分层理念）。仓库实际采用扁平布局：根目录下为 Project/ User/ BSP/ App/
Components/ OSAL/ Services/ Firmware/ Doc/ tools/ .vscode/，目录名与树中编号
一一对应（如 1_BSP→BSP/，3_Components→Components/，6_Core→User/），真实
结构以根目录 README 第 6 节「目录结构」为准。

----------------------------------------------------------------
二、示例例程总览（Example Routine Catalog，共 29 个）
----------------------------------------------------------------
说明：例程按外设类别分组；「代码」列标记该例程是否已提供 main 调用
示例（见第三章）。未提供完整代码的例程仅列出名称与功能说明，
待后续按相同 BSP/APP 接口风格补全。

【1. GPIO 输入输出】
  01_GPIO_Running_LED           流水灯/LED 全功能测试（BSP+APP 两层）      [有代码]
  02_GPIO_Key_Polling_mode     按键轮询模式（GPIO 输入轮询）              [待补]
  03_EXTI_Key_Interrupt_mode   按键外部中断模式（EXTI）                   [有代码]

【2. USART 串口】
  04_USART_Printf              串口 printf 重定向打印                     [待补]
  05_USART_Echo_Interrupt_mode 串口中断回显                              [待补]
  06_USART_DMA                 串口三阶段测试：Printf→IRQ 回显→DMA 回显   [有代码]

【3. ADC 模数转换】
  07_ADC_Temperature_Vrefint   ADC 内部通道：温度 / 内部参考电压          [待补]
  08_ADC0_ADC1_Follow_up_mode  双 ADC 跟随模式（DMA）                     [待补]
  09_ADC0_ADC1_Routine_Parallel_mode  双 ADC 并行模式（DMA）              [有代码]

【4. DAC 数模转换】
  10_DAC_Output_Voltage_Value  DAC 阶梯波电压扫描（PA4 输出）             [有代码]

【5. I2C / SPI 串行总线】
  11_I2C_EEPROM                I2C 读写 EEPROM（AT24C02 等）             [待补]
  12_SPI_Quad_Flash            SPI 四线 Flash（GD25Qxx，硬件/软件 SPI）   [有代码]

【6. 音频 / 显示】
  13_I2S_Audio_Player          I2S 音频播放（WAV 测试数据）              [有代码]
  24_TLI_IPA                  TLI 液晶显示 + IPA 图像处理加速            [待补]
  25_DCI_OV2640               DCI 摄像头 OV2640 采集                    [待补]

【7. 外部存储器（EXMC / SDIO）】
  14_EXMC_SDRAM               EXMC 外扩 SDRAM                           [有代码]
  15_EXMC_SDRAM_DeepSleep     EXMC SDRAM + 深度睡眠                     [有代码]
  16_EXMC_NandFlash           EXMC NAND Flash                          [有代码]
  17_SDIO_SDCardTest          SDIO SD 卡读写测试                        [待补]

【8. 通信总线】
  18_CAN_Network             CAN 总线网络                              [待补]
  27_ENET                    以太网（ETH）                             [待补]
  28_USB_Device              USB 设备                                 [待补]
  29_USB_Host                USB 主机                                 [待补]

【9. 系统 / 定时器 / 其他外设】
  19_RCU_Clock_Out           RCU 时钟输出（MCO）                       [待补]
  20_CTC_Calibration        CTC 时钟校准                              [待补]
  21_PMU_Sleep_Wakeup       PMU 睡眠与唤醒                            [待补]
  22_RTC_Calendar           RTC 实时时钟日历                          [待补]
  23_TIMER_Breath_LED       TIMER 呼吸灯（PWM）                       [待补]
  26_TRNG_Get_Random        TRNG 硬件随机数                           [待补]

注：14/15/16（EXMC）由默认 main.c 的「阶段 2 存储测试」覆盖；12/13 见
第三章对应自检示例。11_I2C_EEPROM 已有 app_i2c_eeprom 组件，但尚未单独
放在本文档的 main 示例里（可参照 README 第 4.2 节手动切换）。

----------------------------------------------------------------
三、主函数调用示例（Main Function Calling Examples）
----------------------------------------------------------------
以下为已提供完整 main() 的例程，直接对应第二章标记 [有代码] 的条目。
代码中的 BSP/APP 接口（如 bsp_led_xxx / app_led_xxx / bsp_usart_xxx
/ bsp_adc_xxx / app_adc_xxx / bsp_dac_xxx / bsp_spi_xxx / bsp_i2s_xxx
/ drv_storage_run / drv_gd25qxx_xxx / drv_i2s_player_xxx）即为第一章架构
中 1_BSP、3_Components、5_App 层对外暴露的调用函数。

注：本节末尾附「当前工程默认 main.c」——即仓库实际烧录运行的
GD32F470 Full Test（阶段 1 按键轮询 → 阶段 2 EXMC 存储测试 → EXTI 中断
模式），与根目录 README 第 4.1 / 5.2 节描述的默认例程一致；其上的
01 / 03 / 06 / 09 / 10 / 12 / 13 为可切换的示例例程，需替换 main.c 主体后
重新编译。

================================================================
01_GPIO_Running_LED
================================================================
/*!
    \brief      系统主函数 - 专测 LED (BSP层 + APP层)
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    printf("\r\n========================================\r\n");
    printf("LED Ultimate Test Starts\r\n");
    printf("========================================\r\n");

    printf("[BSP Test 1] bsp_led_init (Init 3 LEDs)\r\n");
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);
    delay_1ms(500);

    printf("[BSP Test 2] bsp_led_on (Turn on one by one)\r\n");
    bsp_led_on(LED1); delay_1ms(300);
    bsp_led_on(LED2); delay_1ms(300);
    bsp_led_on(LED3); delay_1ms(500);

    printf("[BSP Test 3] bsp_led_off (Turn off one by one)\r\n");
    bsp_led_off(LED1); delay_1ms(300);
    bsp_led_off(LED2); delay_1ms(300);
    bsp_led_off(LED3); delay_1ms(500);

    printf("[BSP Test 4] bsp_led_toggle (Toggle one by one)\r\n");
    bsp_led_toggle(LED1); delay_1ms(300);
    bsp_led_toggle(LED2); delay_1ms(300);
    bsp_led_toggle(LED3); delay_1ms(300);
    bsp_led_toggle(LED1); delay_1ms(300);
    bsp_led_toggle(LED2); delay_1ms(300);
    bsp_led_toggle(LED3); delay_1ms(300);
    delay_1ms(500);

    printf("[APP Test 1] app_led_all_on (All On)\r\n");
    app_led_all_on();
    delay_1ms(1000);

    printf("[APP Test 2] app_led_all_toggle (All Toggle)\r\n");
    app_led_all_toggle();
    delay_1ms(1000);

    printf("[APP Test 3] app_led_all_off (All Off)\r\n");
    app_led_all_on();
    delay_1ms(500);
    app_led_all_off();
    delay_1ms(1000);

    printf("[APP Test 4] app_led_flash (Flash 3 times)\r\n");
    app_led_flash(3);
    delay_1ms(500);

    printf("[APP Test 5] app_led_flow_task (Run flow once)\r\n");
    app_led_flow_task();
    delay_1ms(1000);

    printf("========================================\r\n");
    printf("All LED functions tested OK! Enter while(1).\r\n");
    printf("========================================\r\n");

    while(1) {
    }
}

================================================================
03_EXTI_Key_Interrupt_mode
================================================================
/*!
    \brief      系统主函数 - 按键阶段测试 (先全轮询，后全中断)
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);

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

    while(1) {
    }
}

================================================================
06_USART_DMA
================================================================
/*!
    \brief      系统主函数 - 串口终极测试 (先测 Printf，再测 IRQ，最后测 DMA)
*/
int main(void) {
    uint8_t rx_buf[256];
    uint16_t rx_len = 0;

    systick_config();
    bsp_led_init(LED1);

    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

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

    while(1) {
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0';
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            bsp_usart_send_dma(BSP_USART_COM0, rx_buf, rx_len);
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }
}

================================================================
09_ADC0_ADC1_Routine_Parallel_mode
================================================================
/*!
    \brief      系统主函数 - ADC 三种模式轮番测试
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

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
    while(1) {
        delay_1ms(1000);
    }
}

================================================================
10_DAC_Output_Voltage_Value
================================================================
/*!
    \brief      系统主函数 - DAC 阶梯波扫描测试
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    printf("\r\n========================================\r\n");
    printf("DAC PA4 Scan Test Started!\r\n");
    printf("Outputting 0V -> 3.3V -> 0V continuously.\r\n");
    printf("Measure PA4 with a multimeter to see the change.\r\n");
    printf("========================================\r\n");

    bsp_dac_init();

    while(1) {
        app_dac_scan_task();
        delay_1ms(10);
    }
}

================================================================
12_SPI_Quad_Flash（GD25Qxx 硬件/软件 SPI 双模自检）
================================================================
/*!
    \brief      系统主函数 - SPI Flash 双模自检（硬件 SPI + 软件 SPI）
    说明：SPI5 接 GD25Qxx 系列 Flash；依次以硬件/软件 SPI 探测、擦扇区、
           Quad 写读 512 字节并比对。需 wave_data 无关，仅需 Flash。
*/
#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
#include "bsp_spi.h"
#include "drv_gd25qxx.h"
#include <string.h>

static gd25qxx_dev_t s_flash;

static uint8_t xfer(uint8_t t) { return bsp_spi_transfer(BSP_SPI_BUS_FLASH, t); }
static void    csl(void)       { bsp_spi_cs_low(BSP_SPI_BUS_FLASH); }
static void    csh(void)       { bsp_spi_cs_high(BSP_SPI_BUS_FLASH); }
static void    qr(void)        { bsp_spi_quad_enter_read(BSP_SPI_BUS_FLASH); }
static void    qw(void)        { bsp_spi_quad_enter_write(BSP_SPI_BUS_FLASH); }
static void    qx(void)        { bsp_spi_quad_exit(BSP_SPI_BUS_FLASH); }

#define TEST_ADDR   0x000000u
#define TEST_SIZE   512u

static int flash_test(bsp_spi_mode_t mode)
{
    static uint8_t tx[TEST_SIZE];
    static uint8_t rx[TEST_SIZE];
    const char *name = (mode == BSP_SPI_MODE_HW) ? "HARDWARE" : "SOFTWARE";
    int use_quad;

    printf("\r\n========== %s SPI test ==========\r\n", name);

    bsp_spi_init_mode(BSP_SPI_BUS_FLASH, mode);
    use_quad = bsp_spi_supports_quad(BSP_SPI_BUS_FLASH);

    gd25qxx_bind(&s_flash, xfer, csl, csh,
                 use_quad ? qr : NULL,
                 use_quad ? qw : NULL,
                 use_quad ? qx : NULL);

    if (gd25qxx_probe(&s_flash) != GD25QXX_OK) {
        printf("[%s] probe FAILED, id=0x%06lX\r\n", name, (unsigned long)s_flash.id);
        return -1;
    }
    printf("[%s] id=0x%06lX, cap=%luKB, quad=%s\r\n",
           name, (unsigned long)s_flash.id,
           (unsigned long)(s_flash.capacity / 1024),
           use_quad ? "yes" : "no");

    if (use_quad) gd25qxx_quad_enable(&s_flash);

    for (uint32_t i = 0; i < TEST_SIZE; i++) tx[i] = (uint8_t)i;
    memset(rx, 0, sizeof(rx));

    int rc = gd25qxx_erase_sector(&s_flash, TEST_ADDR);
    if (rc != GD25QXX_OK) {
        printf("[%s] erase FAILED, rc=%d\r\n", name, rc);
        return rc;
    }

    rc = gd25qxx_quad_write(&s_flash, tx, TEST_ADDR, TEST_SIZE);
    if (rc != GD25QXX_OK) {
        printf("[%s] write FAILED, rc=%d\r\n", name, rc);
        return rc;
    }

    rc = gd25qxx_quad_read(&s_flash, rx, TEST_ADDR, TEST_SIZE);
    if (rc != GD25QXX_OK) {
        printf("[%s] read FAILED, rc=%d\r\n", name, rc);
        return rc;
    }

    if (memcmp(tx, rx, TEST_SIZE) != 0) {
        printf("[%s] verify FAILED\r\n", name);
        return -2;
    }

    printf("[%s] PASSED\r\n", name);
    return 0;
}

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    printf("\r\n===== GD25Qxx dual-mode self test =====\r\n");

    int r_hw = flash_test(BSP_SPI_MODE_HW);
    delay_1ms(10);
    int r_sw = flash_test(BSP_SPI_MODE_SW);

    printf("\r\n===== RESULT =====\r\n");
    printf("  HARDWARE SPI : %s\r\n", r_hw == 0 ? "PASS" : "FAIL");
    printf("  SOFTWARE SPI : %s\r\n", r_sw == 0 ? "PASS" : "FAIL");

    while (1) {
        delay_1ms(1000);
    }
}

================================================================
13_I2S_Audio_Player（WAV 测试数据播放，硬件/软件 I2S）
================================================================
/*!
    \brief      系统主函数 - I2S 播放器自检
    说明：以硬件/软件 I2S 播放 wave_data.c 内嵌的 wavetestdata 测试音。
*/
#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
#include "bsp_i2s.h"
#include "drv_i2s_player.h"
#include "wave_data.h"

static drv_i2s_player_t s_player;

static void test_hw(void)
{
    printf("\r\n---- HARDWARE I2S ----\r\n");
    bsp_i2s_init_mode(BSP_I2S_BUS_AUDIO, BSP_I2S_MODE_HW);

    int rc = drv_i2s_player_play_blocking(&s_player, wavetestdata, wavetestdata_len);
    if (rc != 0) {
        printf("[HW] FAILED rc=%d\r\n", rc);
        return;
    }
    printf("[HW] rate=%lu ch=%u bits=%u frames=%lu\r\n",
           (unsigned long)s_player.wav.sample_rate,
           s_player.wav.num_channels,
           s_player.wav.bits_per_sample,
           (unsigned long)s_player.wav.total_frames);
    printf("[HW] PASS\r\n");
}

static void test_sw(void)
{
    printf("\r\n---- SOFTWARE I2S ----\r\n");
    bsp_i2s_init_mode(BSP_I2S_BUS_AUDIO, BSP_I2S_MODE_SW);

    int rc = drv_i2s_player_play_blocking(&s_player, wavetestdata, wavetestdata_len);
    if (rc != 0) {
        printf("[SW] FAILED rc=%d\r\n", rc);
        return;
    }
    printf("[SW] PASS\r\n");
}

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);
    nvic_irq_enable(BSP_I2S_AUDIO_IRQn, 0, 1);

    drv_i2s_player_init(&s_player, BSP_I2S_BUS_AUDIO);

    printf("\r\n===== I2S Player Self Test =====\r\n");
    printf("WAV: %lu bytes\r\n", (unsigned long)wavetestdata_len);

    test_hw();
    delay_1ms(500);
    test_sw();

    printf("\r\n===== ALL DONE =====\r\n");

    while (1) { delay_1ms(1000); }
}

================================================================
当前工程默认 main.c（GD32F470 Full Test：按键轮询 → EXMC 存储 → EXTI）
================================================================

int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);

    printf("\r\n========================================\r\n");
    printf("GD32F470 Full Test Boot\r\n");
    printf("========================================\r\n");

    /* =========================================================
     * 阶段一：按键轮询测试 (5 秒)
     * ========================================================= */
    printf("\r\n[STAGE 1] POLLING TEST (5 seconds)...\r\n");
    printf("Press PA0/PC13/PB14 to toggle LEDs.\r\n");

    app_key_init_polling();
    for (uint32_t i = 0; i < 5000; i++) {
        app_key_polling_task();
        delay_1ms(1);
    }

    /* =========================================================
     * 阶段二：EXMC 三项存储测试 (SDRAM -> SLEEP -> NAND)
     * ========================================================= */
    printf("\r\n[STAGE 2] EXMC STORAGE TEST...\r\n");
    
    for (int m = 0; m < STORAGE_MODE_MAX; m++) {
        drv_storage_run((storage_mode_t)m);
        delay_1ms(1000);
    }

    /* =========================================================
     * 阶段三：大满贯结束，按键功能恢复
     * ========================================================= */
    printf("\r\n========================================\r\n");
    printf("ALL TESTS COMPLETED SUCCESSFULLY!\r\n");
    printf("You can still press keys to toggle LEDs.\r\n");
    printf("========================================\r\n");

    /* 把按键切回中断模式，死循环里按啥亮啥 */
    app_key_init_exti(); 
    
    while (1) {
        delay_1ms(1000);
    }
}