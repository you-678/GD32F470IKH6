## 项目简介

基于 **GD32470I-EVAL 评估板（GD32F470IKH6，Cortex-M4 @ 200MHz）** 的 Keil MDK-ARM 裸机单工程。把官方分散 Demo 抽成 `BSP / App / User` 分层并引入 `Components/drivers` 组件层，引脚与时钟由 `User/bsp_pin_def.h` 集中管理，验证「构建 → 烧录 → BSP → App」全链路打通。

v0.1.0 的默认程序是 **全外设串联自检 Harness**：上电后依次跑 ① LED → ② KEY（轮询 + EXTI）→ ③ USART（printf 心跳 + IRQ 回显 + DMA 回显）→ ④ ADC（内部温度 / Vref / Vbat + 双 ADC DMA 跟随 / 并行）→ ⑤ DAC 阶梯波 → ⑥ SPI Flash（硬 / 软双模 Quad 比对）→ ⑦ I2S（硬 / 软双模播放 WAV）→ ⑧ EXMC（SDRAM / 深睡唤醒 / NAND），最后打印 `ALL TESTS COMPLETED SUCCESSFULLY!`。用 `RUN_xx` 编译开关与 `AUTO_RUN_ALL` 可精简或单测。

## 🚀 核心功能 (v0.1.0)

### 1. EXMC 外部存储（SDRAM + NAND）

- **统一总线切换**：`bsp_exmc` 支持在 SDRAM 与 NAND 之间切换、复用底层数据线；`drv_storage_run()` 统一调度 `SDRAM` / `SDRAM-SLEEP` / `NAND` 三种模式。
- **深睡眠数据保持**：进入深睡前令 SDRAM 自刷新，PA0 按键唤醒后重配时钟并重新初始化 SDRAM，校验掉电前数据，实测通过（日志 `Woke up! Clock restored.` → `Re-init SDRAM...` → `[SDRAM-SLEEP] PASS`）。

### 2. 高阶低功耗与唤醒

- **原生 WFI 唤醒**：使用原生 `__WFI()` + `SCB_SCR_SLEEPDEEP_Msk` 进入深睡，避开官方库封装导致的假死问题。
- **无损时钟恢复**：唤醒后 `bsp_exmc_restore_clock()` 恢复 200MHz 主频、重配 FMC 等待周期与串口，杜绝串口乱码。
- **按键中断唤醒**：复用 `bsp_key` / `app_key` 的 EXTI 框架，PA0 下降沿唤醒。

### 3. I2S 音频 & SPI Flash 双模

- **I2S 硬 / 软双模**：`BSP_I2S_MODE_HW` 与 `BSP_I2S_MODE_SW`，配合 `drv_wav` 解析 + `drv_i2s_player` 播放嵌入的 `wavetestdata`（8000Hz / 单声道 / 16bit / 8000 帧）。
- **SPI 硬 / 软双模 + Quad**：硬件 SPI 与 GPIO 模拟 SPI 动态切换，探测 GD25Qxx ID / 容量并做 512 字节擦写读比对；软件模式无 io2/io3 引脚时自动降级单线（`quad=no`）。
- **I2C / AT24C02**：提供硬件 I2C 与软件模拟 I2C 两套实现、AT24C02 驱动与 `app_i2c_eeprom` 自检；**默认 Harness 未接入**，需按 README §4.2 手动启用。

### 4. 其余已跑通模块

LED、KEY、USART0（printf / IRQ / DMA 回显）、ADC（内部通道 + 双 ADC DMA）、DAC（PA4 阶梯波）。逐项真实实现与关键文件见 README 第 2 节。

## 快速使用

1. Keil MDK-ARM 5.40 打开 `GD32F470IKH6/Project/F470IKH6_demo.uvprojx`，F7 编译、F8 下载。
2. JP5 短接 1-2，USB-TTL 接 COM0（PA9 / PA10），串口助手设 **115200 / 8-N-1 / 无流控**。
3. 复位后观察八项自检日志；EXMC 深睡阶段会打印 `>>> PRESS PA0 TO WAKEUP <<<`，按 PA0 继续。
4. 实板建议：跑深睡唤醒测试时断开调试器、由外部 5V 独立供电（实测经验，非代码硬性要求）。

> 本仓库通过 `.gitignore` 忽略全部构建产物，clone 后须本地编译；Release 附带的 `F470IKH6_demo.hex` 为早前提交的构建产物，建议按当前源码重新编译获取最新固件。
