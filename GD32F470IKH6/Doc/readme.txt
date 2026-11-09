01_GPIO_Running_LED

int main(void)
{
    systick_config();
    bsp_led_init();

    while(1)
    {
        led_flow_task();
    }
}
现象：三灯以 1 秒/颗的节奏做 LED1→LED2→LED3 循环流水，周期 3 秒；
但函数内是阻塞延时，执行期间 CPU 被独占 3 秒，主循环里其它任务（尤其串口 DMA 轮询）会被明显拖慢；
且第 1 段漏关 LED2，在非理想初态下会出现"双灯同亮 1 秒"的异常。

02_GPIO_Key_Polling_mode

int main(void)
{
    systick_config();
    bsp_led_init();
    bsp_key_init();

    while(1)
    {
        key_scan_task();
    }
}
现象：三个按键（Wakeup/Tamper/User）按下后分别翻转 LED1/LED2/LED3，每个按键都带 100ms 消抖且用 `while` 死等松手；
一旦某个键长按不放，程序就卡在该 `while` 里出不来，此时其它按键、LED 流水和串口 DMA 任务全部被阻塞——"按一下就变一下灯、长按就整机僵住"。

03_EXTI_Key_Interrupt_mode

int main(void)
{
    systick_config();

    bsp_led_init();
    bsp_key_exti_init();

    app_led_flash(1);  // 上电闪烁一次自检

    while(1)
    {
        /* 主循环空，全部由中断驱动LED翻转 */
    }
}
现象：三个按键走 EXTI 硬件中断，按下瞬间分别翻转 LED1（PA0/EXTI0）、LED2（PC13/EXTI13）、LED3（PB14/EXTI14），响应无需轮询、无延时消抖，
但也没有硬件/软件消抖和松手处理——"按一下中断里翻一次灯，机械抖动和长按都可能被当成多次按下导致灯反复翻转、难以稳定停在期望状态"。

04_USART_Printf

int main(void)
{
    systick_config();
    bsp_led_init();
	bsp_key_init();
    app_led_flash(2);   //上电LED闪烁两次，和原demo保持一致

    app_usart_init();

    while(1)
    {
        app_usart_task();
    }
}
现象：上电 LED 闪 2 次 → 串口初始化 → 主循环只轮询 TAMPER 键（PC13）：
按住期间 LED3 常亮并不断 `printf("USART printf example")`（长按会疯狂刷屏），
松开立即灭灯——"按住才亮、按住就一直打印，其余 LED 流水和 Wakeup/User 键完全没跑"。

05_USART_Echo_Interrupt_mode

int main(void)
{
    systick_config();
    bsp_led_init();
    app_led_flash(1);

    app_usart_irq_init();
    app_usart_irq_task();

    while(1)
    {
		
    }
}
现象：上电闪一次灯 → 初始化串口中断 → 死等"TX 发完"再死等"RX 收完"（任意一方不齐就永久卡死）→ 比对通过进入三灯 200ms 节拍无限流水、失败则无限闪灯；
app_usart_irq_task() 自带 while(1) 永不返回，main 里的主循环形同虚设。

06_USART_DMA

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