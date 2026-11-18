## 项目简介

基于 **GD32470I-EVAL 评估板（GD32F470IKH6，Cortex-M4 @ 200MHz）** 的 Keil MDK-ARM 裸机单工程。把官方分散 Demo 抽成 `BSP / App / User` 分层并引入 `Components/drivers` 组件层，引脚 / 时钟 / AF / DMA 由 `User/bsp_pin_def.h` 集中管理，验证「构建 → 烧录 → BSP → App」全链路打通；换板只改 `bsp_pin_def.h` 一个文件。

**默认程序是 TRNG 硬件随机数测试**：上电以 IRQ 模式初始化 COM0（115200），初始化片内 TRNG，主循环跑 `app_usart_task()`（按 `\r`/`\n` 行断帧）。串口发 `TRNG` 打印两个 0~255 的硬件随机数，发 `BAUD9600` 运行时切 9600，其它字符串 `[IRQ RX] Got:` 原样回显并 DMA 发回。

## 🚀 核心功能

### 1. 默认 `main()` 实际会跑的

- **USART0 IRQ 模式**：`bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U)`，RBNE 中断收 256B 缓冲，`fputc` 重定向 `printf`。
- **TRNG 真随机数**：`bsp_trng_init()`（等 `DRDY`，失败重试 3 次）、`bsp_trng_random_get()` 取 32 位随机数、`bsp_trng_random_range_get(min,max)` 取区间值。
- **按行断帧命令行**：`app_usart_task()` 只在收到 `\r`/`\n` 时处理整行，早期「长串分包」现象已消除。
- **启动打印**：`Gigadevice TRNG test` 横幅、`TRNG init ok`、`Send 'TRNG' to generate random number.`

### 2. 已编译进固件、需手动切 `main()` 的模块

下表全部已加入 `F470IKH6_demo.uvprojx` 编译，但默认 `main()` 不调用；需按 `README.md` §4.2 替换 `main()` 主体才会运行。

| 类别 | 模块 |
| --- | --- |
| 基础外设 | LED / KEY（轮询 + EXTI）、ADC（内部温度 / Vref / Vbat + 双 ADC DMA）、DAC（PA4 阶梯波） |
| 串行总线 | I2C + AT24C02（硬 / 软双模）、SPI + GD25Qxx（硬 / 软 + Quad，SW 无 io2/io3 自动降级单线） |
| 音频 / 存储 | I2S + WAV Player（硬 / 软双模，8000Hz / mono / 16bit）、EXMC SDRAM 32MB + NAND 16MB（深睡自刷新 + PA0 唤醒时钟恢复） |
| 通信 / 时钟 | SDIO SD 卡、CAN0、RCU Clock-out、CTC 校准、RTC 日历、PMU 睡眠、TIMER PWM 呼吸灯 |
| 显示 / 图像 | TLI + IPA + LCD（12 张 `image*.h` 图层混合）、DCI + OV2640（SCCB 配置 + DMA1_CH7 采集显示） |
| 工程基础 | SysTick 1kHz、集中引脚宏、31 个 `gd32f4xx_*.c` 标准库、`SDIO_IRQHandler` / `DMA1_Channel7_IRQHandler` |

### 3. 工程化基础

- 工具链：Keil µVision V5.40 / MDK-ARM 5.40 / **ARMCLANG V6.19（AC6，C99，-O2）** / `GigaDevice.GD32F4xx_DFP 3.2.0`。
- 分层：`BSP/` 板级驱动、`App/` 业务逻辑、`Components/drivers/` 芯片无关组件、`User/` 工程配置；所有 `.c` 已在工程文件组内。
- 许可：自有代码 MIT（`LICENSE`）；`Firmware/` 下 GigaDevice 官方库遵循其文件头 BSD-3-Clause。

## 快速使用

1. Keil MDK-ARM 5.40 打开 `GD32F470IKH6/Project/F470IKH6_demo.uvprojx`，F7 编译、F8 下载。
2. JP5 短接 1-2，USB-TTL 接 COM0（PA9 / PA10），串口助手设 **115200 / 8-N-1 / 无流控**。
3. 复位后看到 `TRNG init ok`，发送 `TRNG<回车>` 得到两个随机数；发 `BAUD9600<回车>` 后把串口助手同步改成 9600。
4. 默认 `main` 未初始化 LED / KEY / SysTick，上电不点灯、按键无响应是**真实行为**，不是故障。

## 固件与文档说明

> ⚠️ 附件 `F470IKH6_demo.hex` 是 **2026-11-13 旧版构建**（114 KB，对应早前的「全外设串联自检 Harness」默认 `main`），**与当前 tag 内的 TRNG 源码不一致**，仅作历史参考。请按当前源码本地重新编译烧录；`.gitignore` 忽略全部构建产物，仓库本身不含固件。

- 当前真实功能、接线、排错、输入输出示例：根目录 `README.md`（2026-11-18 版，§1–§8）。
- 29 例程框架 + 历史 Harness 完整代码：`Doc/readme.txt` 第二、三章（第三章标题已标注「历史参考」）。
- 当前默认 TRNG `main.c` 全文与版本演进：`Doc/readme.txt` 第四章。
- 详细发布核对清单：`relnotes_v010.md`。

## 已知限制

- 29 个例程中 `27_ENET`、`28/29_USB` 仍为标题占位；`Firmware/GD32F4xx_usb_library/` 未加入编译（`uvprojx` 引用数 0），仅 COM0 可用。
- `OSAL/`、`Services/`、`Components/protocols`、`Components/third_party` 为预留空目录。
- `BSP/`、`App/` 下有若干 `*.bak_*` 调试备份，不参与编译。
- 部分源码（`main.c` / `app_*.c` / `bsp_*.c`）中文注释为 GBK；`Doc/readme.txt` 与根目录 `.md` 文档均为 UTF-8。用 GBK / GB18030 打开源码不影响编译。
