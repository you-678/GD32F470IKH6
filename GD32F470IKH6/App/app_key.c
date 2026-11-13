#include "app_key.h"

/* ================= 中断回调（极简，绝不能加 printf） ================= */
static void on_wakeup_exti(key_typedef_enum keynum) {
    (void)keynum;
    bsp_led_toggle(LED1);
}

static void on_tamper_exti(key_typedef_enum keynum) {
    (void)keynum;
    bsp_led_toggle(LED2);
}

static void on_user_exti(key_typedef_enum keynum) {
    (void)keynum;
    bsp_led_toggle(LED3);
}

/* ================= 初始化函数 ================= */
void app_key_init_polling(void) {
    bsp_key_init(KEY_WAKEUP, KEY_MODE_GPIO);
    bsp_key_init(KEY_TAMPER, KEY_MODE_GPIO);
    bsp_key_init(KEY_USER,   KEY_MODE_GPIO);
}

void app_key_init_exti(void) {
    bsp_key_init(KEY_WAKEUP, KEY_MODE_EXTI);
    bsp_key_register_callback(KEY_WAKEUP, on_wakeup_exti);
    
    bsp_key_init(KEY_TAMPER, KEY_MODE_EXTI);
    bsp_key_register_callback(KEY_TAMPER, on_tamper_exti);
    
    bsp_key_init(KEY_USER, KEY_MODE_EXTI);
    bsp_key_register_callback(KEY_USER, on_user_exti);
}

/* ================= 轮询任务（可以用 printf） ================= */
void app_key_polling_task(void) {
    if (bsp_key_state_get(KEY_WAKEUP) == 0) {
        delay_1ms(20);
        if (bsp_key_state_get(KEY_WAKEUP) == 0) {
            bsp_led_toggle(LED1);
            printf("[Polling] PA0 Pressed! LED1 Toggle.\r\n");
            while(bsp_key_state_get(KEY_WAKEUP) == 0);
        }
    }
    if (bsp_key_state_get(KEY_TAMPER) == 0) {
        delay_1ms(20);
        if (bsp_key_state_get(KEY_TAMPER) == 0) {
            bsp_led_toggle(LED2);
            printf("[Polling] PC13 Pressed! LED2 Toggle.\r\n");
            while(bsp_key_state_get(KEY_TAMPER) == 0);
        }
    }
    if (bsp_key_state_get(KEY_USER) == 0) {
        delay_1ms(20);
        if (bsp_key_state_get(KEY_USER) == 0) {
            bsp_led_toggle(LED3);
            printf("[Polling] PB14 Pressed! LED3 Toggle.\r\n");
            while(bsp_key_state_get(KEY_USER) == 0);
        }
    }
}