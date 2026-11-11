#include "app_led.h"

/*!
    \brief      点亮所有指示灯
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_all_on(void) { 
    bsp_led_on(LED1); 
    bsp_led_on(LED2); 
    bsp_led_on(LED3); 
}

/*!
    \brief      熄灭所有指示灯
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_all_off(void) { 
    bsp_led_off(LED1); 
    bsp_led_off(LED2); 
    bsp_led_off(LED3); 
}

/*!
    \brief      翻转所有指示灯状态
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_all_toggle(void) { 
    bsp_led_toggle(LED1); 
    bsp_led_toggle(LED2); 
    bsp_led_toggle(LED3); 
}

/*!
    \brief      所有指示灯闪烁指定次数 (阻塞式)
    \param[in]  times: 闪烁次数
    \param[out] 无
    \retval     无
*/
void app_led_flash(uint8_t times) {
    for(uint8_t i = 0; i < times; i++) {
        delay_1ms(500); 
        app_led_all_on();
        delay_1ms(500); 
        app_led_all_off();
    }
}

/*!
    \brief      指示灯流水灯任务 (阻塞式)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_led_flow_task(void) {
    bsp_led_on(LED1); 
    bsp_led_off(LED3); 
    delay_1ms(1000);
    
    bsp_led_on(LED2); 
    bsp_led_off(LED1); 
    delay_1ms(1000);
    
    bsp_led_on(LED3); 
    bsp_led_off(LED2); 
    delay_1ms(1000);
}