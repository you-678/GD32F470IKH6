#include "app_key.h"

/* =========================================================
 * 内部私有函数：EXTI 中断回调
 * ========================================================= */

/*!
    \brief      PA0 按键的中断回调函数
    \param[in]  keynum: 触发中断的按键编号
    \param[out] 无
    \retval     无
*/
static void on_wakeup_exti(key_typedef_enum keynum) {
    (void)keynum;
    bsp_led_toggle(LED1);
    printf("[EXTI Interrupt] PA0 Pressed! LED1 Toggle.\r\n");
}

/*!
    \brief      PC13 按键的中断回调函数
    \param[in]  keynum: 触发中断的按键编号
    \param[out] 无
    \retval     无
*/
static void on_tamper_exti(key_typedef_enum keynum) {
    (void)keynum;
    bsp_led_toggle(LED2);
    printf("[EXTI Interrupt] PC13 Pressed! LED2 Toggle.\r\n");
}

/*!
    \brief      PB14 按键的中断回调函数
    \param[in]  keynum: 触发中断的按键编号
    \param[out] 无
    \retval     无
*/
static void on_user_exti(key_typedef_enum keynum) {
    (void)keynum;
    bsp_led_toggle(LED3);
    printf("[EXTI Interrupt] PB14 Pressed! LED3 Toggle.\r\n");
}

/* =========================================================
 * 对外暴露的 APP 层接口
 * ========================================================= */

/*!
    \brief      APP层按键初始化 (配置为轮询模式)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_key_init_polling(void) {
    bsp_key_init(KEY_WAKEUP, KEY_MODE_GPIO);
    bsp_key_init(KEY_TAMPER, KEY_MODE_GPIO);
    bsp_key_init(KEY_USER,   KEY_MODE_GPIO);
}

/*!
    \brief      APP层按键初始化 (配置为中断模式并注册回调)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_key_init_exti(void) {
    bsp_key_init(KEY_WAKEUP, KEY_MODE_EXTI);
    bsp_key_register_callback(KEY_WAKEUP, on_wakeup_exti);
    
    bsp_key_init(KEY_TAMPER, KEY_MODE_EXTI);
    bsp_key_register_callback(KEY_TAMPER, on_tamper_exti);
    
    bsp_key_init(KEY_USER, KEY_MODE_EXTI);
    bsp_key_register_callback(KEY_USER, on_user_exti);
}

/*!
    \brief      APP层按键轮询任务 (带消抖和松手检测，需在主循环不断调用)
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_key_polling_task(void) {
    /* 检测 PA0 */
    if (bsp_key_state_get(KEY_WAKEUP) == 0) {
        delay_1ms(20);
        if (bsp_key_state_get(KEY_WAKEUP) == 0) {
            bsp_led_toggle(LED1);
            printf("[Polling] PA0 Pressed! LED1 Toggle.\r\n");
            while(bsp_key_state_get(KEY_WAKEUP) == 0);
        }
    }
    /* 检测 PC13 */
    if (bsp_key_state_get(KEY_TAMPER) == 0) {
        delay_1ms(20);
        if (bsp_key_state_get(KEY_TAMPER) == 0) {
            bsp_led_toggle(LED2);
            printf("[Polling] PC13 Pressed! LED2 Toggle.\r\n");
            while(bsp_key_state_get(KEY_TAMPER) == 0);
        }
    }
    /* 检测 PB14 */
    if (bsp_key_state_get(KEY_USER) == 0) {
        delay_1ms(20);
        if (bsp_key_state_get(KEY_USER) == 0) {
            bsp_led_toggle(LED3);
            printf("[Polling] PB14 Pressed! LED3 Toggle.\r\n");
            while(bsp_key_state_get(KEY_USER) == 0);
        }
    }
}