#include "app_rtc.h"

/* 阻塞等待串口输入两位数字 (为了与例程保持一致) */
static uint8_t wait_for_number(void) {
    uint8_t buf[2];
    uint16_t len = 0;
    uint8_t step = 0;
    
    while(step < 2) {
        if (bsp_usart_get_rx_data(BSP_USART_COM0, &buf[step], &len)) {
            if (buf[step] >= '0' && buf[step] <= '9') {
                step++;
            } else {
                printf("\r\nPlease input a valid number (0-9)\r\n");
            }
        }
    }
    /* 将输入的字符 '12' 转换为 BCD 码 0x12 */
    return (uint8_t)((buf[0] - 0x30) << 4) | (buf[1] - 0x30);
}

void app_rtc_init(void)
{
    printf("\r\n  ****************** RTC calendar demo ******************\r\n");

    /* 1. 初始化 RTC 底层硬件 */
    bsp_rtc_init();

    /* 2. 检查是否已经配置过时间 */
    if (!bsp_rtc_is_configured()) {
        printf("\r\n[RTC] First time running, please configure time.\r\n");
        
        printf("Please input hour (00-23): \r\n");
        uint8_t hour = wait_for_number();
        
        printf("Please input minute (00-59): \r\n");
        uint8_t minute = wait_for_number();
        
        printf("Please input second (00-59): \r\n");
        uint8_t second = wait_for_number();

        /* 3. 设置时间 */
        bsp_rtc_set_time(hour, minute, second);
    } else {
        printf("\r\n[RTC] RTC is already configured.\r\n");
    }

    /* 4. 清除复位标志 */
    rcu_all_reset_flag_clear();

    /* 初次运行打印一次时间 */
    bsp_rtc_show_time();
}

void app_rtc_task(void)
{
    static uint8_t last_sec = 0xFF;
    rtc_parameter_struct local_time;

    /* 获取当前时间 */
    rtc_current_time_get(&local_time);
    
    /* 如果秒数发生变化，就打印一次 */
    if (local_time.second != last_sec) {
        last_sec = local_time.second;
        bsp_rtc_show_time();
    }

    /* 加个极小的延时，防止主循环跑得太快把串口挤爆 */
    delay_1ms(100);
}