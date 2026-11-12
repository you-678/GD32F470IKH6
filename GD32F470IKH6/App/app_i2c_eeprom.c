#include "app_i2c_eeprom.h"

static uint8_t s_ok    = 0;
static uint8_t s_count = 0;

void app_i2c_eeprom_init(bsp_i2c_mode_enum mode)
{
    bsp_led_init(LED1);
    bsp_led_init(LED2);
    bsp_led_init(LED3);
    bsp_led_off(LED1);
    bsp_led_off(LED2);
    bsp_led_off(LED3);

    bsp_i2c_init(mode);

    printf("\r\nThe I2C is %s interface ",
           (mode == BSP_I2C_MODE_HW) ? "hardware" : "software");
    printf("\r\nThe speed is 100000");

    printf("\r\nbefore at24c02_init\r\n");
    uint8_t probe = at24c02_init();
    printf("at24c02_init ret = %u\r\n", probe);

    printf("before at24c02_test\r\n");
    uint8_t test_ret = at24c02_test();
    printf("at24c02_test ret = %u\r\n", test_ret);

    if (test_ret == 0) {
        printf("24C02 test OK!\r\n");
        s_ok = 1;
    } else {
        printf("24C02 test FAIL!\r\n");
        s_ok = 0;
    }
}

void app_i2c_eeprom_task(void)
{
    if (s_ok) {
        bsp_led_off(LED1);
        bsp_led_off(LED2);
        bsp_led_off(LED3);
        switch (s_count % 3) {
        case 0: bsp_led_on(LED1); break;
        case 1: bsp_led_on(LED2); break;
        case 2: bsp_led_on(LED3); break;
        default: break;
        }
        s_count++;
        if (s_count >= 3) s_count = 0;
        delay_1ms(500);
    } else {
        bsp_led_on(LED1);
        bsp_led_on(LED2);
        bsp_led_on(LED3);
    }
}