#ifndef __APP_I2C_EEPROM_H
#define __APP_I2C_EEPROM_H

#include "bsp_i2c.h"

void app_i2c_eeprom_init(bsp_i2c_mode_enum mode);
void app_i2c_eeprom_task(void);

#endif