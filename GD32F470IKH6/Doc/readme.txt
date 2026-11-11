01_GPIO_Running_LED
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

02_GPIO_Key_Polling_mode


03_EXTI_Key_Interrupt_mode

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

04_USART_Printf


05_USART_Echo_Interrupt_mode


06_USART_DMA


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

07_ADC_Temperature_Vrefint

08_ADC0_ADC1_Follow_up_mode

09_ADC0_ADC1_Routine_Parallel_mode

10_DAC_Output_Voltage_Value

11_I2C_EEPROM

12_SPI_Quad_Flash

13_I2S_Audio_Player

14_EXMC_SDRAM

15_EXMC_SDRAM_DeepSleep

16_EXMC_NandFlash

17_SDIO_SDCardTest

18_CAN_Network

19_RCU_Clock_Out

20_CTC_Calibration

21_PMU_Sleep_Wakeup

22_RTC_Calendar

23_TIMER_Breath_LED

24_TLI_IPA

25_DCI_OV2640

26_TRNG_Get_Random

27_ENET

28_USB_Device

29_USB_Host