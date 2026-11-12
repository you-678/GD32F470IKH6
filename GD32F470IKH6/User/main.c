#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"
//#include "gd32f470i_eval.h"

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    /* ¸Ä BSP_I2C_MODE_SW ¼´ÇÐÈí¼þ I2C */
    app_i2c_eeprom_init(BSP_I2C_MODE_HW);

    while (1) {
        app_i2c_eeprom_task();
    }
}
