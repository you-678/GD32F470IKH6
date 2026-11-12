================================================================
GD32F470IKH6 项目文档 (Doc/readme.txt)
================================================================
说明：本文档汇总项目的「分层架构」与「官方示例例程（29 个）」。
其中架构树与 main 调用函数部分原样保留，其余例程已按外设类别
条理清晰地编排说明。

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
 │    │    ├── bsp_spi.h       // (预留 Flash/屏幕)
 │    │    └── bsp_adc.h / bsp_dac.h
 │    └── src/
 │         └── (对应的 .c 文件)
 │
 ├── 2_OSAL/                   // [系统抽象] 操作系统适配层
 │    ├── inc/
 │    │    └── os_wrapper.h    // 提供 os_delay, os_mutex, os_task_create
 │    └── src/
 │         ├── os_baremetal.c  // 裸机下用软件定时器模拟
 │         └── os_freertos.c   // (后期上 FreeRTOS 时加进来)
 │
 ├── 3_Components/             // [组件层] 核心算法、协议解析、第三方设备驱动
 │    ├── drivers/             //  传感器/外设驱动 (芯片无关)
 │    │    ├── sensor_def.h    // 传感器统一接口结构体 (C语言面向对象)
 │    │    ├── drv_sht30.c/h    // 温湿度
 │    │    ├── drv_mpu6050.c/h  // 六轴
 │    │    └── drv_at24c02.c/h  // EEPROM
 │    ├── protocols/           //  通信协议
 │    │    └── parser_custom.c/h  // 处理粘包、校验（给上位机用）
 │    └── third_party/         // 第三方开源库
 │         ├── cJSON/          // (处理JSON通信)
 │         └── fatfs/          // (文件系统)
 │
 ├── 4_Services/               // [服务层] 系统级业务管理中心
 │    ├── inc/
 │    │    ├── svc_sensor.h    // 传感器管理 (统一调度、总线防冲突)
 │    │    ├── svc_comm.h      // 通信管理 (打包/解包、上报下发)
 │    │    ├── svc_param.h     // 参数管理 (读写 EEPROM)
 │    │    └── svc_ota.h       // OTA升级管理
 │    └── src/
 │         └── (对应的 .c 文件)
 │
 ├── 5_App/                    // [应用层] 纯业务逻辑 (完全不碰硬件)
 │    ├── inc/
 │    │    ├── app_main.h      // 总入口
 │    │    ├── app_task_env.h  // 环境监测任务
 │    │    └── app_task_ui.h   // 界面显示任务
 │    └── src/
 │         └── (对应的 .c 文件)
 │
 ├── 6_Core/                   // [核心层] 启动与中断
 │    ├── main.c               // 极简主函数 (只调 App 层初始化)
 │    ├── gd32f4xx_it.c        // 所有中断服务函数入口
 │    └── systick.c            // 系统心跳
 │
 ├── 7_Project/                // Keil 工程文件存放处 (.uvprojx 等)
 │
 └── 8_Doc/                    // 文档 (README, 硬件引脚表, 通信协议手册)

注：以上为「规划中的分层目录」示意（编号 0~8 对应 BSP/App/Components 等
分层理念）。仓库实际采用扁平布局：根目录下为 Project/ User/ BSP/ App/
Components/ OSAL/ Services/ Firmware/ Doc/ .vscode/，目录名与树中编号一一
对应（如 1_BSP→BSP/，3_Components→Components/，6_Core→User/），真实结构
以根目录 README 第 6 节「目录结构」为准。

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
  05_USART_Echo_Interrupt_mode 串口中断回显                               [待补]
  06_USART_DMA                 串口三阶段测试：Printf→IRQ 回显→DMA 回显   [有代码]

【3. ADC 模数转换】
  07_ADC_Temperature_Vrefint   ADC 内部通道：温度 / 内部参考电压          [待补]
  08_ADC0_ADC1_Follow_up_mode  双 ADC 跟随模式（DMA）                     [待补]
  09_ADC0_ADC1_Routine_Parallel_mode  双 ADC 并行模式（DMA）              [有代码]

【4. DAC 数模转换】
  10_DAC_Output_Voltage_Value  DAC 阶梯波电压扫描（PA4 输出）             [有代码]

【5. I2C / SPI 串行总线】
  11_I2C_EEPROM                I2C 读写 EEPROM（AT24C02 等）             [待补]
  12_SPI_Quad_Flash            SPI 四线 Flash（预留屏幕/Flash）           [待补]

【6. 音频 / 显示】
  13_I2S_Audio_Player          I2S 音频播放                              [待补]
  24_TLI_IPA                  TLI 液晶显示 + IPA 图像处理加速            [待补]
  25_DCI_OV2640               DCI 摄像头 OV2640 采集                    [待补]

【7. 外部存储器（EXMC / SDIO）】
  14_EXMC_SDRAM               EXMC 外扩 SDRAM                           [待补]
  15_EXMC_SDRAM_DeepSleep     EXMC SDRAM + 深度睡眠                     [待补]
  16_EXMC_NandFlash           EXMC NAND Flash                          [待补]
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

----------------------------------------------------------------
三、主函数调用示例（Main Function Calling Examples）
----------------------------------------------------------------
以下为已提供完整 main() 的例程，直接对应第二章标记 [有代码] 的条目。
代码中的 BSP/APP 接口（如 bsp_led_xxx / app_led_xxx / bsp_usart_xxx
/ bsp_adc_xxx / app_adc_xxx / bsp_dac_xxx）即为第一章架构中 1_BSP、
5_App 层对外暴露的调用函数。

注：本节末尾另附「当前工程默认 main.c」一段——即仓库实际烧录运行的
两阶段 I2C 自检程序（硬件 I2C → 软件 I2C），与根目录 README 第 4.1 /
5.2 节描述的默认例程一致；其上 01 / 03 / 06 / 09 / 10 为可切换的示例
例程，需替换 main.c 主体后重新编译。

================================================================
01_GPIO_Running_LED
================================================================
/*!
    \brief      系统主函数 - 专测 LED (BSP层 + APP层)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    /* 1. 初始化滴答定时器和串口 (为了打印测试进度) */
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("LED Ultimate Test Starts\r\n");
    printf("========================================\r\n");

    /* =========================================================
     * 第一部分：测试 BSP 底层函数
     * ========================================================= */
    
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
    bsp_led_toggle(LED1); delay_1ms(300); /* On */
    bsp_led_toggle(LED2); delay_1ms(300); /* On */
    bsp_led_toggle(LED3); delay_1ms(300); /* On */
    bsp_led_toggle(LED1); delay_1ms(300); /* Off */
    bsp_led_toggle(LED2); delay_1ms(300); /* Off */
    bsp_led_toggle(LED3); delay_1ms(300); /* Off */
    delay_1ms(500);


    /* =========================================================
     * 第二部分：测试 APP 业务层函数
     * ========================================================= */
    
    printf("[APP Test 1] app_led_all_on (All On)\r\n");
    app_led_all_on();
    delay_1ms(1000);

    printf("[APP Test 2] app_led_all_toggle (All Toggle)\r\n");
    app_led_all_toggle();
    delay_1ms(1000);

    printf("[APP Test 3] app_led_all_off (All Off)\r\n");
    app_led_all_on();   /* On first */
    delay_1ms(500);
    app_led_all_off();  /* Then Off */
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

    /* 停在这里，防止反复重启测试 */
    while(1) {
    }
}

================================================================
03_EXTI_Key_Interrupt_mode
================================================================
/*!
    \brief      系统主函数 - 按键阶段测试 (先全轮询，后全中断)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    /* 1. 基础环境初始化 */
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    bsp_led_init(LED1); 
    bsp_led_init(LED2); 
    bsp_led_init(LED3);

    /* =========================================================
     * 阶段一：全轮询模式 (执行 15 秒)
     * ========================================================= */
    printf("\r\n========================================\r\n");
    printf("STAGE 1: ALL POLLING MODE (15 seconds)\r\n");
    printf("Press PA0, PC13, PB14 to test Polling.\r\n");
    printf("========================================\r\n");

    /* 调用 APP 层初始化轮询 */
    app_key_init_polling();

    /* 运行 15 秒的轮询测试 (每次 1ms，循环 15000 次) */
    for (uint32_t i = 0; i < 15000; i++) {
        app_key_polling_task(); /* 调用 APP 层轮询任务 */
        delay_1ms(1);
    }

    /* =========================================================
     * 阶段二：全中断模式 (死循环)
     * ========================================================= */
    printf("\r\n========================================\r\n");
    printf("STAGE 2: ALL EXTI MODE (Infinite loop)\r\n");
    printf("Press PA0, PC13, PB14 to test EXTI.\r\n");
    printf("========================================\r\n");

    /* 调用 APP 层初始化中断并注册回调 */
    app_key_init_exti();

    /* 进入死循环，硬件中断在后台自动触发，主循环啥也不干 */
    while(1) {
    }
}

================================================================
06_USART_DMA
================================================================
/*!
    \brief      系统主函数 - 串口终极测试 (先测 Printf，再测 IRQ，最后测 DMA)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    uint8_t rx_buf[256];
    uint16_t rx_len = 0;

    /* 1. 基础环境初始化 */
    systick_config();
    bsp_led_init(LED1);

    /* =========================================================
     * 阶段一：纯 Printf 测试 (持续 5 秒)
     * ========================================================= */
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("STAGE 1: Printf Test Started (5 seconds)\r\n");
    printf("You should see heartbeats below.\r\n");
    printf("========================================\r\n");

    /* 循环 50 次，每次 100ms，总计 5 秒 */
    for (int i = 0; i < 50; i++) {
        printf("[Printf] Heartbeat %d\r\n", i);
        delay_1ms(100);
    }

    /* =========================================================
     * 阶段二：中断接收 (IRQ) + 普通回显 (持续 10 秒)
     * ========================================================= */
    /* 重新初始化串口，开启中断接收 */
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("STAGE 2: IRQ RX & Printf Echo (10 seconds)\r\n");
    printf("Please send some data to test IRQ.\r\n");
    printf("========================================\r\n");

    /* 循环 1000 次，每次 10ms，总计 10 秒 */
    for (int i = 0; i < 1000; i++) {
        /* 查询是否收到数据 */
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0'; /* 添加结束符 */
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            bsp_led_toggle(LED1);
        }
        delay_1ms(10);
    }

    /* =========================================================
     * 阶段三：中断接收 (IRQ) + DMA 发送回显 (无限循环)
     * ========================================================= */
    printf("\r\n========================================\r\n");
    printf("STAGE 3: IRQ RX & DMA Echo (Infinite loop)\r\n");
    printf("Please send some data to test DMA echo.\r\n");
    printf("========================================\r\n");

    while(1) {
        if (bsp_usart_get_rx_data(BSP_USART_COM0, rx_buf, &rx_len)) {
            rx_buf[rx_len] = '\0';
            printf("[IRQ RX] Got: %s\r\n", rx_buf);
            
            /* 调用底层 DMA 发送函数，把收到的数据发回去 */
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
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    /* =========================================================
     * 阶段一：测试 内部通道 软件触发模式 (持续 10 秒)
     * ========================================================= */
    bsp_adc_init(BSP_ADC_MODE_INTERNAL);
    printf("\r\n========================================\r\n");
    printf("STAGE 1: INTERNAL ADC (10s)\r\n");
    printf("========================================\r\n");
    
    for (int i = 0; i < 1000; i++) { /* 1000 * 10ms = 10s */
        app_adc_task_internal();
        delay_1ms(10);
    }

    /* =========================================================
     * 阶段二：测试 双ADC 跟随模式 DMA (持续 10 秒)
     * ========================================================= */
    bsp_adc_init(BSP_ADC_MODE_DUAL_DMA_FOLLOW_UP);
    printf("\r\n========================================\r\n");
    printf("STAGE 2: DUAL ADC FOLLOW UP MODE (10s)\r\n");
    printf("========================================\r\n");
    
    for (int i = 0; i < 1000; i++) {
        app_adc_task_dual_dma();
        delay_1ms(10);
    }

    /* =========================================================
     * 阶段三：测试 双ADC 并行模式 DMA (持续 10 秒)
     * ========================================================= */
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
        delay_1ms(1000); /* 测试结束，停在此处 */
    }
}

================================================================
10_DAC_Output_Voltage_Value
================================================================
/*!
    \brief      系统主函数 - DAC 阶梯波扫描测试
    \param[in]  无
    \param[out] 无
    \retval     无
*/
int main(void) {
    /* 1. 初始化基础环境 */
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
    
    printf("\r\n========================================\r\n");
    printf("DAC PA4 Scan Test Started!\r\n");
    printf("Outputting 0V -> 3.3V -> 0V continuously.\r\n");
    printf("Measure PA4 with a multimeter to see the change.\r\n");
    printf("========================================\r\n");

    /* 2. 初始化 DAC 硬件 */
    bsp_dac_init();

    /* 3. 主循环 */
    while(1) {
        /* 调用业务层任务，不断改变电压 */
        app_dac_scan_task();
        
        /* 延时 10ms 作为系统心跳 */
        delay_1ms(10);
    }
}


================================================================
当前工程默认 main.c（两阶段 I2C 自检：硬件 I2C → 软件 I2C）
================================================================
/*!
    \brief      系统主函数 - 当前仓库默认行为（EEPROM 两阶段自检）
    \param[in]  无
    \param[out] 无
    \retval     无
    说明：上电后先以硬件 I2C0（BSP_I2C_MODE_HW）初始化并探测 AT24C02，
          做整片「写 0x00~0xFF → 读回比对」一致性测试，延时 1s 后再以
          软件 I2C（BSP_I2C_MODE_SW，GPIO 模拟 PB6/PB7）跑同样测试，
          最后打印 "Both HW & SW Tested. Entering Loop" 进入死循环。
          循环里 LED 表现由最后一次（SW）测试结果决定：
          通过 → 三灯循环单闪（LED1→LED2→LED3 各 500ms）；
          失败 → 三灯常亮。
          硬件 I2C 与软件 I2C 复用同一总线/器件，通常两者同过或同败。
          本段即为仓库实际烧录运行的 main.c，与根目录 README 第 4.1 / 5.2
          节描述的默认例程一致。
*/
int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    printf("\r\n\r\n");
    printf("========================================\r\n");
    printf("  Stage 1: Test Hardware I2C (HW)       \r\n");
    printf("========================================\r\n");
    
    /* 1. 测试硬件 I2C */
    app_i2c_eeprom_init(BSP_I2C_MODE_HW);
    delay_1ms(1000); /* 稍作延时，避免打印太快 */


    printf("\r\n\r\n");
    printf("========================================\r\n");
    printf("  Stage 2: Test Software I2C (SW)       \r\n");
    printf("========================================\r\n");
    
    /* 2. 测试软件 I2C */
    app_i2c_eeprom_init(BSP_I2C_MODE_SW);
    delay_1ms(1000);


    printf("\r\n\r\n");
    printf("========================================\r\n");
    printf("  Both HW & SW Tested. Entering Loop    \r\n");
    printf("========================================\r\n");

    /* 3. 进入主循环 (跑马灯或全亮，取决于最后一次测试结果) */
    while (1) {
        app_i2c_eeprom_task();
    }
}