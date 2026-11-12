## 项目简介
本工程是基于 GD32F470IKH6 的 Keil MDK-ARM 学习工程，验证 [构建 → 烧录 → BSP → App] 全链路打通。
分层架构（BSP / App / User / Components），`bsp_pin_def.h` 集中管理引脚。

## 当前功能 (v0.1.0)
- **I2C 软/硬双驱动**（`bsp_i2c`），同一套接口切换 `BSP_I2C_MODE_HW` / `BSP_I2C_MODE_SW`
- **AT24C02 驱动**（`drv_at24c02`）：单字节读写、按页写、连续读、全片写读一致性测试
- **EEPROM 例程**（`app_i2c_eeprom`）：上电探测 + 全片测试，串口打印结果，LED 跑马/全亮指示
- **分层架构**：`1_BSP / 2_OSAL / 3_Components / 4_Services / 5_App / 6_Core`

## 快速使用
1. 使用 Keil MDK 打开工程文件。
2. 编译并烧录至 GD32F470IKH6。
3. 串口（PA9/PA10，115200）观察打印结果。
