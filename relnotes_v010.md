## 项目简介
基于 GD32470I-EVAL（GD32F470IKH6，Cortex-M4）的 Keil MDK-ARM 裸机学习工程。
把官方分散 Demo 抽成 BSP / App / User / Components 分层，引入 `bsp_pin_def.h` 集中引脚定义，
验证 [构建 → 烧录 → BSP → App] 全链路打通。

## 默认例程（GD32F470 Full Test）
上电先跑 5s 按键轮询自检（KEY 按下触发 EXTI 中断回调，串口打印事件），
再跑 EXMC 存储测试（SDRAM 读写 → SDRAM 休眠保持 → NAND 读写），
最后切 EXTI 中断模式空闲。详见 README 第 4、5 节。

## 当前功能（v0.1.0 交付范围）
- **LED**：3 路 PE2/PE3/PF10（亮/灭/翻转/流水/闪烁）
- **KEY**：3 键 PA0/PC13/PB14，轮询 + EXTI 中断回调（按下下降沿触发）
- **USART0**：PA9/PA10，printf 重定向 + IRQ 回显 + DMA 发送
- **ADC**：内部通道（温度/参考电压/电池）+ 双 ADC 跟随/并行 DMA
- **DAC**：PA4，12 位阶梯波扫描
- **I2C**：硬件 + 软件双实现，AT24C02 EEPROM 组件驱动
- **SPI**：硬件 SPI + 软件 GPIO 模拟，GD25Qxx Quad SPI Flash 驱动
- **I2S**：硬件 I2S + DMA，WAV 播放器（drv_wav / drv_i2s_player / wave_data）
- **EXMC**：SDRAM / NAND / SLEEP 存储测试（bsp_exmc / drv_storage）
- **组件层**：drivers（AT24C02 / GD25Qxx / WAV / I2S Player / Storage）、wave_data 示例音频

## 工程配置
- Keil MDK-ARM 5.40 / ARMCLANG V6.19（AC6，C99，-O2）
- GigaDevice.GD32F4xx_DFP 3.2.0
- 构建产物需在本地编译，体积随模块增减浮动（以本机实际构建为准）

## 已知限制
- 默认 Full Test 需在 GD32470I-EVAL 实板上运行（依赖 EXMC 外接 SDRAM/NAND）；
  无对应硬件时相关测试会失败，属预期行为。
- OSAL/Services 及 Components/protocols、third_party 为预留空目录。
- Firmware 下 USB 库未加入编译；仅 COM0 可用。
- 随附的 `F470IKH6_demo.hex` 为早前提交构建产物，建议按当前源码本地重新编译获取最新固件。
