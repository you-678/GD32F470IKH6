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

int main(void)
{
    systick_config();
    bsp_led_init(LED1);bsp_led_init(LED2);bsp_led_init(LED3);
    bsp_key_init(KEY_WAKEUP, KEY_MODE_GPIO);bsp_key_init(KEY_TAMPER, KEY_MODE_GPIO);bsp_key_init(KEY_USER, KEY_MODE_GPIO);
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
	
	app_led_flash(2);   //上电LED闪烁两次，和原demo保持一致
	
    while(1)
    {
        app_usart_task();
    }
}
现象：上电 LED 闪 2 次 → 串口初始化 → 主循环只轮询 TAMPER 键（PC13）：
按住期间 LED3 常亮并不断 `printf("USART printf example")`（长按会疯狂刷屏），
松开立即灭灯——"按住才亮、按住就一直打印，其余 LED 流水和 Wakeup/User 键完全没跑"。

05_USART_Echo_Interrupt_mode

/* 全局接收缓冲区和标志位 */
uint8_t  rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile uint8_t  rx_complete_flag = 0;

/* USART0 中断服务函数 */
void USART0_IRQHandler(void)
{
    /* 接收中断 */
    if (usart_interrupt_flag_get(BSP_USART0_PERIPH, USART_INT_FLAG_RBNE) != RESET)
    {
        uint8_t data = (uint8_t)usart_data_receive(BSP_USART0_PERIPH);
        
        if (rx_index < RX_BUFFER_SIZE - 1) {
            rx_buffer[rx_index++] = data;
        }
        
        /* 简单约定：收到 '\n' 认为一帧结束 */
        if (data == '\n') {
            rx_buffer[rx_index] = '\0';   // 字符串结束符
            rx_complete_flag = 1;
        }
    }
    /* 可以在此添加错误中断处理 (USART_INT_FLAG_ERR) */
}
/* 全局接收缓冲区和标志位 */
extern uint8_t  rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_index;
extern volatile uint8_t  rx_complete_flag;

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);  // 中断模式

    while (1)
    {
        if (rx_complete_flag)
        {
            /* 处理收到的完整一帧数据 */
            // 例如：回显收到的字符串
            printf("IRQ Recv: %s", rx_buffer);
            
            /* 清空状态，准备接收下一帧 */
            memset(rx_buffer, 0, sizeof(rx_buffer));
            rx_index = 0;
            rx_complete_flag = 0;
        }
        
        /* 可以在这里执行其他非阻塞任务 */
    }
}
现象：中断模式按字节接收，串口助手发一行（必须以 \n 结尾）才触发一帧——主循环检测到标志后用 printf("IRQ Recv: ...") 回显整行，随后清空缓冲区等下一行；
不发换行就永远不回显，printf 回显期间若再来数据可能被覆盖丢失。

06_USART_DMA

07_ADC_Temperature_Vrefint

int main(void)
{
    systick_config();
    bsp_led_init(LED1);bsp_led_init(LED2);bsp_led_init(LED3);
    bsp_key_init(KEY_WAKEUP, KEY_MODE_GPIO);bsp_key_init(KEY_TAMPER, KEY_MODE_GPIO);bsp_key_init(KEY_USER, KEY_MODE_GPIO);
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);
	bsp_adc_init();
	
	app_led_flash(2);   //上电LED闪烁两次，和原demo保持一致
	
	
    while(1)
    {
        app_adc_task();
    }
}

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