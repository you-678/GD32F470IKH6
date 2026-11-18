# GD32F470IKH6 裸机学习工程

基于 **GD32470I-EVAL 评估板（主控 GD32F470IKH6，Cortex-M4 @ 200MHz）** 的 Keil MDK-ARM 单工程，把官方分散的入门 Demo 抽成 `BSP / App / User` 分层，引入 `Components/drivers` 组件层与 `bsp_pin_def.h` 集中引脚定义，在一个工程里对比 GPIO、串口、ADC/DAC、I2C/SPI/I2S、EXMC 存储、SDIO、CAN、RTC/PMU/CTC、TRNG、DCI 摄像头与 TLI 显示等写法。**换板子只改 `bsp_pin_def.h` 一个文件即可**。

> **当前状态（以 `User/main.c` 为准，commit `9544e11`，2026-11-18）**：默认 `main.c` 已切换为 **TRNG 硬件随机数测试**——上电用 `BSP_USART_MODE_IRQ` 初始化 COM0（115200），初始化 TRNG，然后在主循环里跑 `app_usart_task()`；串口发 `TRNG` 生成两个 0~255 的随机数，发 `BAUD9600` 切到 9600 波特率，其它字符串原样回显。**旧版「全外设串联自检 Harness」已不在默认 `main.c` 中**，其代码作为历史参考保留在 `Doc/readme.txt` 第三章与 v0.1.0 发布说明里。
>
> **重要区分**：`BSP/`、`App/`、`Components/` 下所有 `.c` **都已加入 Keil 工程编译**，但默认 `main()` 只调用 USART(IRQ) + TRNG + `app_usart_task()`。其余模块属于「可用例程」，需按第 4.2 节替换 `main()` 主体才会运行；第 2 节对每一项都标了状态，**不把「编译进固件」误算成「默认会跑」**。

> **文档基准（2026-11-18）**：本文行为描述逐条对照当前工作区源码与工程配置核实，依据为 `User/main.c`、`User/bsp_pin_def.h`、`User/gd32f4xx_libopt.h`、`User/gd32f4xx_it.c`、`App/*.c/.h`、`BSP/*.c/.h`、`Components/drivers/*.c/.h`、`Project/F470IKH6_demo.uvprojx`。复核方法见第 8 节。

---

## 1. 项目解决什么问题

| 痛点 | 本仓库的做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，对比 LED / 按键 / 串口 / ADC / DAC / I2C / SPI / I2S / EXMC / SDIO / CAN / RTC / TRNG / DCI / TLI 时要反复建工程、切工程 | 外设抽成 `BSP/` 板级驱动、业务逻辑抽成 `App/` 任务函数、芯片无关协议抽成 `Components/drivers/`；**只改 `User/main.c` 就能切换例程**，所有 `.c` 已在工程文件组内 |
| 裸机调试没有日志，只能靠点灯猜状态 | `printf` 重定向到 USART0（PA9/PA10，115200-8-N-1），含浮点输出；TRNG 例程直接用串口命令行触发 |
| 换板卡要改一堆散落引脚宏 | 全部引脚/时钟/AF/DMA 宏集中在 `User/bsp_pin_def.h`（含新增 SDIO / CAN0 / SCCB / DCI / TLI 定义），驱动层统一取用 |
| 学习过程中「代码写了、现象没记」 | `Doc/readme.txt` 保留分层架构、29 例程总览与多份完整 `main()` 调用示例，作为历史参考与切换模板 |

**范围边界（避免误读）**：

- 本仓库**不是** 29 个官方例程的移植合集，也不是固件产品或飞控代码，不含 RTOS、协议栈、算法。`Doc/readme.txt` 的 01–29 是例程框架；01/03/06/09/10/12/13/14–16 及新增 17–26 已有对应代码或 `main()` 示例，`27_ENET`、`28/29_USB` 仍为待补标题。
- `Firmware/GD32F4xx_usb_library/` 带官方 USB Device/Host 库源码，但 `uvprojx` 引用数为 0，**未加入编译**；`OSAL/`、`Services/`、`Components/protocols`、`Components/third_party` 为**预留空目录**。
- 验证环境：µVision V5.40.0.0 / MDK-ARM Plus 5.40.0.3 / **ARMCLANG V6.19（AC6，C99，-O2）** / `GigaDevice.GD32F4xx_DFP 3.2.0`。`.gitignore` 忽略全部构建产物（`Objects/`、`Listings/`、`*.o`、`*.hex` 等），**clone 后无固件、无构建日志，须本地 Keil 编译**。
- `BSP/`、`App/` 目录下有若干 `*.bak_*` 调试备份文件（尤其 `app_camera.c`），**不参与编译**，仅为开发过程留档。

---

## 2. 主要功能

### 2.1 默认 `main.c` 实际会跑的（上电即生效）

| 模块 | 真实实现 | 关键文件 |
| --- | --- | --- |
| USART0（IRQ 模式） | `bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U)`；`fputc` 重定向 `printf`；RBNE 中断把字节存入 256B 缓冲 | `BSP/bsp_usart.c/.h` |
| TRNG 真随机数 | `bsp_trng_init()` 使能 `RCU_TRNG` → `trng_deinit/enable` → 等 `DRDY`（失败最多重试 3 次）；`bsp_trng_random_get()` 取 32 位真随机数，`bsp_trng_random_range_get(min,max)` 取区间值 | `BSP/bsp_trng.c/.h` |
| 串口命令行任务 | `app_usart_task()` **按行断帧**（`\r`/`\n` 结尾才处理，256B 行缓冲）：含 `TRNG` → 打印两个随机数；含 `BAUD9600` → 运行时改波特率；其它 → `[IRQ RX] Got: <原串>` 并 `bsp_usart_send_dma()` 回显；每次取到数据调用 `bsp_led_toggle(LED1)`（LED GPIO 未初始化，一般不可见） | `App/app_usart.c` |
| 启动打印 | `Gigadevice TRNG test` 横幅、`TRNG init ok`、`Send 'TRNG' to generate random number.`（失败为 `TRNG init error`） | `User/main.c` |

### 2.2 已编译进工程、需手动切 `main()` 的模块（默认不跑）

> 下表全部**已加入 `F470IKH6_demo.uvprojx` 编译**，状态均为「需按第 4.2 节替换 `main()` 调用」。

| 类别 | 模块 | 真实实现 / 关键 API | 关键文件 |
| --- | --- | --- | --- |
| 基础外设 | LED / KEY | 3 路 LED（PE2/PE3/PF10）`init/on/off/toggle` + `app_led_*`；3 键（PA0/PC13/PB14）轮询 + EXTI 回调、`bsp_key_set_trigger()`（默认 `KEY_TRIG_FALLING_PULLUP`） | `BSP/bsp_led.*`、`BSP/bsp_key.*`、`App/app_led.*`、`App/app_key.*` |
| 基础外设 | ADC / DAC | ADC 三模式（内部温度/Vref/Vbat、双 ADC 跟随 DMA、并行 DMA，TIMER1 触发，DMA1_CH0）；DAC PA4 12 位阶梯波 | `BSP/bsp_adc.*`、`BSP/bsp_dac.*`、`App/app_adc.*`、`App/app_dac.*` |
| 串行总线 | I2C + AT24C02 | 硬件 I2C0（PB6/PB7，AF4）+ 软件模拟双实现，带超时；EEPROM 单字节/分页/连续读写与整片一致性测试 | `BSP/bsp_i2c.*`、`Components/drivers/drv_at24c02.*`、`App/app_i2c_eeprom.*` |
| 串行总线 | SPI + GD25Qxx | SPI5 硬件 + GPIO 模拟双模；Quad 进出、ID/容量探测、512B 擦写读比对；软件模式无 io2/io3 自动降级单线 | `BSP/bsp_spi.*`、`Components/drivers/drv_gd25qxx.*` |
| 音频 | I2S + WAV Player | I2S1 硬件 + 软件双模；`drv_wav` 解析、`drv_i2s_player` 播放嵌入 `wavetestdata`（8000Hz / mono / 16bit） | `BSP/bsp_i2s.*`、`Components/drivers/drv_wav.*`、`drv_i2s_player.*`、`wave_data.*` |
| 存储 | EXMC SDRAM / NAND | SDRAM 32MB @ `0xC0000000`（含自刷新与深睡唤醒时钟恢复）、NAND @ `0x70000000`；`drv_storage_run()` 三模式（SDRAM / SDRAM_SLEEP / NAND） | `BSP/bsp_exmc.*`、`Components/drivers/drv_storage.*` |
| 存储 | SDIO SD 卡 | `bsp_sdio` 命令/响应/DMA 收发（CLK PC12、CMD PD2、D0–D3 PC8–PC11，AF12，DMA1_CH3）；`drv_sdcard` 实现 CMD8/ACMD41 识别流程；`app_sdcard` 打印卡信息、容量并做 512B 读写测试 | `BSP/bsp_sdio.*`、`Components/drivers/drv_sdcard.*`、`App/app_sdcard.*` |
| 通信 | CAN0 | TX PB9 / RX PB8（AF9）；`bsp_can_init(6)`、`bsp_can_send_msg/receive_msg`；`app_can_task()` 按 Tamper 键发 `0x7AB` 8 字节并打印收发数据 | `BSP/bsp_can.*`、`App/app_can.*` |
| 时钟 / 低功耗 | RCU Clock-out | `bsp_clockout_select(seq)` 在 IRC16M / LXTAL / HXTAL 与 PLLI2S 间切换 CK_OUT0/1；`app_clockout_task()` 按 Tamper 键切换 | `BSP/bsp_clockout.*`、`App/app_clockout.*` |
| 时钟 / 低功耗 | CTC / RTC / PMU | CTC 校准 IRC48M（成功点亮 LED1 并打印 `Calibration OK!`）；RTC 日历（首次运行串口输入时分秒，打印 `HH:MM:SS`）；PMU 睡眠，`app_sleep_test()` 进睡前关 SysTick 中断、由串口数据唤醒 | `BSP/bsp_ctc.*`、`bsp_rtc.*`、`bsp_pmu.*`、`App/app_ctc.*`、`app_rtc.*`、`app_sleep.*` |
| 定时器 | TIMER PWM 呼吸灯 | `bsp_timer_pwm_init()`（PB10）、`bsp_timer_pwm_set_duty()`；`app_breath_task()` 5ms 步进占空比 | `BSP/bsp_timer.*`、`App/app_breath.*` |
| 显示 / 图像 | TLI + IPA + LCD | `bsp_tli_init()`、`bsp_tli_blend_layer1_init()`（图层混合）、`bsp_ipa_init_config/start`（图像处理加速）；`app_lcd` 用 `Components/images/` 下 12 张 `image*.h` 做混合显示 | `BSP/bsp_tli.*`、`BSP/bsp_ipa.*`、`App/app_lcd.*`、`Components/images/` |
| 显示 / 图像 | DCI 摄像头 OV2640 | SCCB（I2C0 PB6/PB7，器件写 `0x60` / 读 `0x61`）配置 OV2640；DCI 采集 + DMA1_CH7 搬运到帧缓冲，`bsp_tli` 叠加显示；`app_camera_init()` 注册按键唤醒回调、`app_camera_dma_convert_handler()` 处理转换中断 | `BSP/bsp_dci.*`、`bsp_sccb.*`、`Components/drivers/OV2640/dci_ov2640.*`、`App/app_camera.*` |
| 系统 / 中断 | SysTick + NVIC | `systick_config()` → `delay_1ms()`；`EXTI0`/`EXTI10_15` 分发按键、`USART0_IRQHandler`（在 `bsp_usart.c`）、`SPI1_IRQHandler`（在 `bsp_i2s.c`）、`SDIO_IRQHandler`、`DMA1_Channel7_IRQHandler`（摄像头 DMA） | `User/systick.c`、`User/gd32f4xx_it.c` |
| 官方固件 | CMSIS + 标准外设库 | 25MHz HXTAL 经 PLL 配 200MHz；`gd32f4xx_*.c` 共 31 个加入编译（含 can/ctc/dci/ipa/rtc/sdio/tli/trng 等新增外设库） | `Firmware/CMSIS/`、`Firmware/GD32F4xx_standard_peripheral/` |
| 引脚集中定义 | `bsp_pin_def.h` | LED/KEY/USART/ADC/DAC/I2C/SPI/I2S/EXMC + **SDIO / CAN0 / TLI / SCCB / DCI / OV2640 地址 / SDIO DMA** 宏 | `User/bsp_pin_def.h` |
| 头文件汇总 | `gd32f4xx_libopt.h` | 汇总标准库头 + 全部 `bsp_` / `app_` / `drv_` 头 + `bsp_pin_def.h`（`drv_i2s_player.h` 被注释，由 `main.c` 显式包含） | `User/gd32f4xx_libopt.h` |
| 工程与资料 | Keil / 文档 / 工具 | `F470IKH6_demo.uvprojx`、评估板原理图与用户指南 PDF、`Doc/readme.txt`、`tools/gen_wave.py` | `Project/`、`.vscode/`、`Doc/`、`tools/` |

**状态图例**：`默认 main 调用` = 上电就会跑（仅 2.1）；`已编译进工程` = 源码在固件里但默认不调用（2.2 全部）；`待补` = 只有标题、无代码（`27_ENET`、`28/29_USB`）。

> 注 1：USART 只有 `BSP_USART_MODE_PRINTF` 与 `BSP_USART_MODE_IRQ` 两种模式；旧 `BSP_USART_MODE_DMA` 已移除，DMA 仅用于 `bsp_usart_send_dma()` 发送，接收走 IRQ。
> 注 2：`app_usart_task()` 现已按行断帧，早期 README 里描述的「长串被切成两半的分包现象」已不复现。

---

## 3. 安装方法

### 3.1 软件环境

| 项 | 要求 |
| --- | --- |
| 操作系统 | Windows（Keil µVision 工程，无 Makefile / CMake 构建链） |
| IDE / 编译器 | Keil MDK-ARM 5.40（本机 5.40.0.3），ARMCLANG V6.19（AC6）；工程已勾选 `uAC6`、`C99`、优化 `-O2` —— 其他 AC6 版本未验证 |
| 器件支持包 | `GigaDevice.GD32F4xx_DFP 3.2.0`；工程选定器件 `GD32F470IK`，IROM 3MB @ `0x08000000`，Flash 算法 `GD32F4xx_3MB.FLM` |
| 串口终端 | 任意支持 115200 / 8-N-1 / 无流控的串口助手（**需能发送带 `\r\n` 的文本**） |
| 可选 | VS Code + Keil Assistant 插件（`.vscode/` 已带 IntelliSense 配置） |

### 3.2 硬件与接线

- **板卡**：GD32470I-EVAL（GD32F470IKH6），Mini USB 或 DC-005 提供 5V。**默认 TRNG 例程只用到片内 TRNG 与 USART0，不依赖板载 SDRAM / NAND / SD 卡 / 摄像头**；切换其它例程时才需要对应外设。
- **下载**：调试适配器配置为 CMSIS-DAP「Any」/ SW-DP（`CMSIS_AGDI.dll`）；板载 GD-Link 或外置 J-Link / CMSIS-DAP 均可，需在 `Options for Target → Debug` 里选定。
- **串口（默认例程必需）**：USART0 = PA9(TX) / PA10(RX)。评估板经 MAX3232 引出到 DB9 **COM0**，**必须用跳线帽把 JP5 短接 1-2 选择 USART0**（短接 2-3 是 USB_FS）。COM0 是 **RS232 电平**，请用 USB-RS232 串口线；若用 USB-TTL，请直接接 PA9 / PA10 / GND 的 **TTL 电平**，不要接 DB9（来源：用户指南 V2.0 第 15 页原理图、第 30 页串口参数）。
- **基础引脚表**（与 `bsp_pin_def.h` 一致，用户指南 V2.0 表 2-1）：

| 功能 | 引脚 | 功能 | 引脚 |
| --- | --- | --- | --- |
| LED1 | PE2 | Wakeup 键 | PA0 |
| LED2 | PE3 | Tamper 键 | PC13 |
| LED3 | PF10 | User 键 | PB14 |
| USART0_TX | PA9 | USART0_RX | PA10 |
| ADC_CH13 | PC3 | ADC_CH15 | PC5 |
| DAC_OUT | PA4 | I2C_SCL / SDA | PB6 / PB7 |
| SPI_FLASH_SCK / MISO / MOSI | PG13 / PG12 / PG14 | SPI_FLASH_IO2 / IO3 / CS | PG10 / PG11 / PI8 |
| I2S_MCK / BCK / WS / SD | PA6 / PI1 / PI0 / PC1 | EXMC SDRAM / NAND | 由 `bsp_exmc.c` 固定配置 |

- **切换例程时新增的接线**（宏定义见 `User/bsp_pin_def.h`）：

| 例程 | 引脚 / 说明 |
| --- | --- |
| SDIO SD 卡 | CLK=PC12、CMD=PD2、D0–D3=PC8–PC11，AF12；DMA1_CH3（SUBPERI4）；`SDIO_IRQHandler` 已在 `gd32f4xx_it.c` |
| CAN0 | TX=PB9、RX=PB8，AF9，中断 `CAN0_RX0_IRQn`；`bsp_can_init(6)` 为波特率预分频 |
| SCCB / OV2640 | SCL=PB6、SDA=PB7（I2C0，AF4），器件写地址 `0x60`、读地址 `0x61` |
| DCI 摄像头 | PIXCLK=PA6、VSYNC=PG9 等（详见 `bsp_pin_def.h` DCI 段）；DMA1_CH7 全传输中断在 `gd32f4xx_it.c` |
| TLI LCD | RGB 数据线与控制线（AF14），详见 `bsp_pin_def.h` TLI 段；图层缓冲在片外 SDRAM |
| TIMER 呼吸灯 | PB10 PWM 输出 |
| EXMC | SDRAM @ `0xC0000000`（32MB）、NAND @ `0x70000000`，板载自带 |

### 3.3 获取与编译

```bash
git clone https://github.com/you-678/GD32F470IKH6.git
cd GD32F470IKH6
```

1. 用 µVision 打开 **`GD32F470IKH6/Project/F470IKH6_demo.uvprojx`**（仓库根目录和源码目录同名，是两层 `GD32F470IKH6/`，别只进第一层；Target：`GD32F470IKH6_demo`）。
2. `Project → Build Target`（F7）。产物在 `GD32F470IKH6/Project/Objects/`：`F470IKH6_demo.axf`、`F470IKH6_demo.hex`、`F470IKH6_demo.build_log.htm`、`F470IKH6_demo.htm`。**本工程未生成 `.map` 文件**。
3. `Download`（F8，或把 `.hex` 烧进 Flash）后复位运行。

> `.gitignore` 忽略 `Objects/`、`Listings/`、`*.o`、`*.d`、`*.hex`、`*.map`，**clone 后仓库里没有固件，必须自行编译**。

### 3.4 常见报错与排错

| 现象 | 处理 |
| --- | --- |
| `#error "Please select the target GD32F4xx device in gd32f4xx.h file"` | `Firmware/CMSIS/GD/GD32F4xx/Include/gd32f4xx.h` 的器件宏被注释，本机由 DFP 注入；换环境后在 `Options for Target → C/C++ (AC6) → Define` 添加 `GD32F470` |
| 串口无输出 | 依次查：JP5 是否短接 1-2、是否接 COM0、终端是否 115200-8-N-1、是否误把 RS232 口接 USB-TTL；默认 `main.c` 上电即打印 TRNG 横幅，与按键无关 |
| 打印 `TRNG init error` / `Error: TRNG can't ready` | 确认 `gd32f4xx_trng.c` 已加入工程编译、`RCU_TRNG` 时钟可用；`bsp_trng_init()` 内部最多重试 3 次，仍失败会卡在 `while(1)` |
| 发 `TRNG` 没反应 | ① 必须以 `\r` 或 `\n` 结尾（`app_usart_task()` 按行断帧）；② 串口必须初始化成 `BSP_USART_MODE_IRQ`；③ 发送 ASCII 文本、区分大小写；④ 每收到数据 LED1 会翻转，可据此判断中断是否进来了 |
| 发 `BAUD9600` 后乱码 | 这是预期行为：MCU 已切到 9600，需把串口助手也改成 9600；复位可回到 115200 |
| 温度/电压打印 `0.000` | 仅当改用 **Microlib** 时需要；在 `Options for Target → C/C++ (AC6)` 勾选「Use float with printf」再编译。本工程用 ARMCLANG 标准库，`%f` 默认可用 |
| 双 ADC 读数恒 0 / 乱跳 | 内部通道无需接线；双 ADC 模式需 PC3、PC5 接真实模拟信号，悬空读到噪声 |
| SD 卡 `[SD] Card init failed!` | 检查 SDIO 引脚接线、卡座供电与卡型；`drv_sdcard` 会打印 CMD8 / ACMD41 响应便于定位 |
| 摄像头无图像 | 依次查 SCCB 能否读到 OV2640 ID、DCI 引脚、`DMA1_Channel7_IRQHandler` 是否启用、TLI 图层是否 `enable` + `reload`；该模块有多个 `*.bak_before_*` 备份，属在调状态 |
| 中文注释乱码 | **部分源码**（`main.c`、`app_*.c`、`bsp_*.c` 的注释）是 GBK 编码，用 GBK / GB18030 打开即可；`Doc/readme.txt` 与根目录 `.md` 文档为 **UTF-8**，勿用 GBK 打开。乱码不影响编译 |
| VS Code Keil Assistant 报「发现无效的文件组」 | 插件解析 `OSAL/`、`Services/` 等空目录告警，不影响 µVision 编译 |

---

## 4. 使用方法

### 4.1 跑通默认例程（TRNG 硬件随机数测试）

1. JP5 短接 1-2，串口线接 COM0，串口助手设 **115200 / 8 数据位 / 1 停止位 / 无校验 / 无流控**，并开启「发送追加 `\r\n`」。
2. 编译（F7）→ 下载（F8）→ 复位。
3. 上电应看到：

   ```
    /==============Gigadevice TRNG test=============/
   TRNG init ok
   Send 'TRNG' to generate random number.
   ```

4. 在串口助手发送 **`TRNG`**（带回车），返回两个 0~255 的硬件随机数：

   ```
   [TRNG] random1 = 137, random2 = 42
   ```

5. 其它指令：发 `BAUD9600` 切到 9600 波特率；发任意其它字符串会回显 `[IRQ RX] Got: <原串>` 并由 DMA 原样发回。每收到一次数据，代码会调用 `bsp_led_toggle(LED1)`，但默认 `main` 未初始化 LED GPIO，通常看不到灯变化。

> `main.c` 里还保留了一段**被注释掉的 SD 卡测试 `main()`**（`app_sdcard_init/config/info_print/test`），需要时取消注释并注释掉当前 TRNG 的 `main()` 即可。

### 4.2 切换到其它例程

当前 `main.c` 只有 51 行，**没有 `RUN_xx` / `AUTO_RUN_ALL` 编译开关了**（那是 v0.1.0 Harness 的写法，已归档到 `Doc/readme.txt`）。切换方式是**替换 `main()` 主体**：

1. 打开 `GD32F470IKH6/Doc/readme.txt` 第三章，找到目标例程的完整 `main()` 片段。
2. 复制进 `User/main.c`，替换现有 `main()`（或注释掉现有的）。
3. 按例程要求调整串口模式：多数例程用 `BSP_USART_MODE_PRINTF`；**RTC / Sleep / TRNG 必须用 `BSP_USART_MODE_IRQ`**（要收串口数据）。
4. F7 重新编译 → F8 下载。

可直接复用的 `main()` 示例（`Doc/readme.txt` 第三章，均已编译进工程）：

| 例程 | 入口 | 备注 |
| --- | --- | --- |
| 01 流水灯 / 03 EXTI 按键 / 06 串口 DMA / 09 双 ADC / 10 DAC / 12 SPI Quad Flash / 13 I2S WAV | `app_led_*` / `app_key_*` / `app_usart_task()` / `app_adc_*` / `app_dac_scan_task()` / `drv_gd25qxx_*` / `drv_i2s_player_*` | v0.1.0 老例程 |
| 14–16 EXMC 存储 | `drv_storage_run()` 三模式 | 依赖板载 SDRAM / NAND |
| 17 SDIO SD 卡 | `app_sdcard_init/config/info_print/test()` | 需插卡，使能 `SDIO_IRQn` |
| 18 CAN 网络 | `app_can_init/task()` | 按 Tamper 键发帧 |
| 19 RCU Clock-out | `app_clockout_init/task()` | 按 Tamper 键切时钟源，示波器测 CK_OUT |
| 20 CTC 校准 | `app_ctc_init/task()` | 成功点亮 LED1 |
| 21 PMU 睡眠唤醒 | `app_sleep_test()` | **必须 IRQ 模式串口**，发任意数据唤醒 |
| 22 RTC 日历 | `app_rtc_init/task()` | **必须 IRQ 模式**，首次运行串口输入时分秒 |
| 23 TIMER 呼吸灯 | `app_breath_init/task()` | PB10 PWM |
| 24 TLI / IPA / LCD | `app_lcd_init/task()` | 依赖片外 SDRAM 图层缓冲 |
| 25 DCI OV2640 摄像头 | `app_camera_init()` | 需使能 `DMA1_Channel7_IRQn`，按键唤醒采集 |
| 26 TRNG | **当前默认** | 见 4.1 |
| 历史 Harness（八项串联自检） | `RUN_xx` + `Test_xx()` 全套代码 | **仅归档**，非当前默认，见 `Doc/readme.txt` 第三章「历史参考」段 |

### 4.3 串口两种模式与命令行（默认例程）

`bsp_usart_init()` 第 2 个参数：

| 模式 | 代码做了什么 | 当前用途 |
| --- | --- | --- |
| `BSP_USART_MODE_PRINTF` | 仅轮询发送，`printf` 重定向到 USART0；接收使能但无中断 | 多数只打印的例程（LED / ADC / SPI / I2S / SD 卡 / CAN / LCD 等） |
| `BSP_USART_MODE_IRQ` | 使能 RBNE 中断 + NVIC；`USART0_IRQHandler`（在 `bsp_usart.c`）把字节存入 256B 缓冲，`app_usart_task()` 按行取出处理 | ✅ **当前默认 main 使用**；TRNG / RTC / Sleep 等需要收数据的例程 |

`app_usart_task()` 的命令行（**按行断帧，`\r`/`\n` 结尾才处理**）：

| 发送内容（含子串匹配） | 响应 |
| --- | --- |
| `TRNG` | `[TRNG] random1 = <0~255>, random2 = <0~255>` |
| `BAUD9600` | `Changing baudrate to 9600...` → `[Success] Baudrate is now 9600 bps.` |
| 其它任意字符串 | `[IRQ RX] Got: <原串>` + `bsp_usart_send_dma()` 原样回显 |

> **串口号不可随便改**：`usart_typedef_enum` 只有 `BSP_USART_COM0`（其后 `BSP_USART_COMn = 1` 为计数），`bsp_usart_init()` 首行 `if (usart_num >= BSP_USART_COMn) return;`。**传其他值会静默不初始化**，要扩展需补齐 `bsp_usart.c` 的查表数组。

### 4.4 常用接口速查

```c
/* 默认 main 用到的 (BSP/bsp_usart.h, BSP/bsp_trng.h, App/app_usart.h) */
void     bsp_usart_init(usart_typedef_enum com, usartmode_typedef_enum mode, uint32_t baudrate);
void     bsp_usart_set_baudrate(usart_typedef_enum com, uint32_t baudrate);
void     bsp_usart_send_dma(usart_typedef_enum com, uint8_t *data, uint16_t len);
uint8_t  bsp_usart_get_rx_data(usart_typedef_enum com, uint8_t *data, uint16_t *len);
ErrStatus bsp_trng_init(void);
ErrStatus bsp_trng_ready_check(void);
uint32_t bsp_trng_random_get(void);
uint8_t  bsp_trng_random_range_get(uint8_t min, uint8_t max);
void     app_usart_task(void);

/* LED / KEY / 系统 */
void bsp_led_init/on/off/toggle(led_typedef_enum lednum);
void bsp_key_init(key_typedef_enum k, keymode_typedef_enum mode);
void bsp_key_set_trigger(key_trigger_typedef_enum trigger);
uint8_t bsp_key_state_get(key_typedef_enum k);          /* 0 = 按下 */
void systick_config(void);  void delay_1ms(uint32_t ms);

/* ADC / DAC / I2C / SPI / I2S / EXMC */
void bsp_adc_init(bsp_adc_mode_enum mode);  void bsp_adc_trigger(void);
uint16_t bsp_adc_get_idata0/1/2(void);     uint32_t bsp_adc_get_dma_data0/1(void);
void bsp_dac_init(void);      void bsp_dac_set_value(uint16_t value);
void bsp_i2c_init(bsp_i2c_mode_enum mode);
uint8_t bsp_i2c_write_bytes/read_bytes(uint8_t dev, uint8_t reg, uint8_t *p, uint16_t len);
void bsp_spi_init_mode(bsp_spi_bus_t bus, bsp_spi_mode_t mode);
uint8_t bsp_spi_transfer(bsp_spi_bus_t bus, uint8_t tx);
void bsp_i2s_init_mode(bsp_i2s_bus_t bus, bsp_i2s_mode_t mode);
void bsp_i2s_start(bsp_i2s_bus_t bus, uint32_t sr_hz, bsp_i2s_tx_cb_t cb, void *ctx, uint32_t num_halfwords);
void bsp_exmc_init_mode(bsp_exmc_mode_t mode);  void bsp_exmc_restore_clock(void);
int  drv_storage_run(storage_mode_t mode);      /* SDRAM / SDRAM_SLEEP / NAND */

/* 新增模块（v0.1.0 之后） */
void     bsp_sdio_init(void);
void     bsp_sdio_send_cmd(uint8_t cmd, uint32_t arg, bsp_sdio_resp_typedef_enum resp);
uint32_t bsp_sdio_get_response(uint8_t index);
void     bsp_sdio_dma_tx_config(uint32_t *src, uint32_t bytes);
void     bsp_sdio_dma_rx_config(uint32_t *dst, uint32_t bytes);
void     app_sdcard_init/config/test/info_print(void);
void     bsp_can_init(uint32_t baudrate_prescaler);
uint8_t  bsp_can_send_msg(bsp_can_msg_t *msg);  uint8_t bsp_can_receive_msg(bsp_can_msg_t *msg);
void     app_can_init/task(void);
void     bsp_clockout_init(void);  void bsp_clockout_select(uint8_t seq);
void     bsp_ctc_init(void);       uint8_t bsp_ctc_is_trim_ok(void);
void     bsp_rtc_init(void);       void bsp_rtc_show_time(void);
void     bsp_rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second);
void     bsp_pmu_init(void);       void bsp_enter_sleep_mode(void);
void     bsp_timer_pwm_init(void); void bsp_timer_pwm_set_duty(uint16_t duty);
void     bsp_sccb_init(void);
uint8_t  bsp_sccb_write(uint8_t reg, uint8_t data);
uint8_t  bsp_sccb_read(uint8_t reg, uint8_t *data);
void     bsp_dci_init(void);       void bsp_tli_init(void);
void     bsp_tli_blend_layer1_init(uint32_t mem_addr, uint16_t width, uint16_t height);
void     bsp_ipa_init_config(uint32_t src, uint32_t dst, uint16_t w, uint16_t h);
void     bsp_ipa_start(void);
void     app_camera_init(void);    void app_camera_dma_convert_handler(void);
void     app_lcd_init/task(void);  void app_breath_init/task(void);
void     app_sleep_test(void);     void app_rtc_init/task(void);
void     app_ctc_init/task(void);  void app_clockout_init/task(void);

/* 引脚集中定义 (User/bsp_pin_def.h)：基础外设 + SDIO/CAN0/TLI/SCCB/DCI/OV2640 全在此 */
```

这些声明由 `User/gd32f4xx_libopt.h` 统一汇总（标准外设库头、`stdio.h`/`string.h`、`systick.h`、全部 `bsp_`/`app_`/`drv_` 头与 `bsp_pin_def.h`），`gd32f4xx.h` 会自动包含它，因此各 `.c` 只要 `#include "gd32f4xx.h"` 就能拿到全部声明（`drv_i2s_player.h` 例外，由 `main.c` 显式包含）。

---

## 5. 输入输出示例

### 5.1 输入

| 输入 | 形式 | 代码路径 / 说明 |
| --- | --- | --- |
| 上电 / 复位 | 硬件 | `bsp_usart_init(COM0, IRQ, 115200)` → `bsp_trng_init()` → `while(1) app_usart_task()`；不初始化 LED / SysTick（默认 main 未调用 `systick_config()`） |
| 串口下发 `TRNG\r\n` | 文本，**必须以 `\r` 或 `\n` 结尾** | `app_usart_task()` 按行缓冲，`strstr(line_buf,"TRNG")` 命中后调用两次 `bsp_trng_random_range_get(0,255)` |
| 串口下发 `BAUD9600\r\n` | 文本 | 命中后 `bsp_usart_set_baudrate(COM0, 9600U)` |
| 串口下发其它字符串 | 文本 | `[IRQ RX] Got: <原串>` + `bsp_usart_send_dma()` 回显；每收到数据调用 `bsp_led_toggle(LED1)`（LED GPIO 未初始化，一般不可见） |
| （切换例程后）按键 PA0 / PC13 / PB14 | 低电平按下 / 上拉 | `app_key_polling_task()` 20ms 消抖或 EXTI 下降沿回调；CAN / Clockout 例程用 Tamper 键 |
| （切换例程后）PC3 / PC5 模拟信号 | 电压 | 双 ADC 模式由 TIMER1 触发、DMA1_CH0 搬运 |
| （切换例程后）PA4 测量端 | 万用表/示波器 | DAC 阶梯波 0~3.3V |
| （切换例程后）SD 卡 / SPI Flash / I2S / OV2640 / CAN 总线 | 硬件 | 见第 4.2 节与 `Doc/readme.txt` 对应 `main()` 片段 |
| （切换例程后）板载 SDRAM / NAND | 外部存储 | `drv_storage_run()` 三模式读写验证；TLI / DCI 图层缓冲也依赖 SDRAM |

### 5.2 输出（默认 `main.c` TRNG 测试的真实串口日志）

> 115200-8-N-1，GD32470I-EVAL 板。随机数由片内 TRNG 产生，每次不同；`…` 表示重复交互。

```
 /==============Gigadevice TRNG test=============/
TRNG init ok
Send 'TRNG' to generate random number.

[TRNG] random1 = 137, random2 = 42          <- 发送 "TRNG\r\n"

[TRNG] random1 = 208, random2 = 15          <- 再发一次
...

[IRQ RX] Got: hello                          <- 发送其它字符串，DMA 原样回显
hello

Changing baudrate to 9600...                 <- 发送 "BAUD9600\r\n"
[Success] Baudrate is now 9600 bps.          <- 此后需把终端切到 9600
```

初始化失败时（TRNG 时钟/种子异常）会是另一条路径：

```
 /==============Gigadevice TRNG test=============/
Error: TRNG can't ready
Clock error current status: 0
Seed error current status: 0
TRNG init fail, retry 1
...
TRNG init error                              <- 3 次重试仍失败，卡在 while(1)
```

### 5.3 现象对照速查（默认 `main.c` = TRNG 测试）

| 操作 | 预期 LED | 预期串口 |
| --- | --- | --- |
| 上电复位 | 无变化（默认 main 未初始化 LED GPIO，仅 `app_usart_task()` 里会 `bsp_led_toggle(LED1)`） | TRNG 横幅 + `TRNG init ok` + 提示语 |
| 发送 `TRNG` + 回车 | 灯无变化（LED GPIO 未初始化） | `[TRNG] random1 = x, random2 = y` |
| 发送普通字符串 + 回车 | 灯无变化（LED GPIO 未初始化） | `[IRQ RX] Got: <原串>` + DMA 回显原串 |
| 发送 `BAUD9600` + 回车 | 灯无变化（LED GPIO 未初始化） | 切波特率提示，之后终端需同步切 9600 |
| 发送不带回车的文本 | 灯无变化（LED GPIO 未初始化） | 无输出，等收到 `\r`/`\n` 才处理（行缓冲未结束） |
| 按板上任意按键 | 无变化 | 无输出（默认 main 未初始化 KEY） |

### 5.4 切换例程后的输出样例（摘自各模块 `printf`）

| 例程 | 典型输出 |
| --- | --- |
| SD 卡 | `[SD] Card init success, capacity=...KB` / `[SD] Card information:` / `## Card version 2.00 ##`；底层 `[SD] CMD8 r7=... resp=0x...`、`[SD] ACMD41 cnt=... resp=...` |
| CAN | `[CAN] Init OK. Press Tamper key to TX.` → `[CAN] Transmit data: a0 a1 ...` / `[CAN] Receive data: ...` |
| RTC | `RTC calendar demo` → 首次 `[RTC] First time running, please configure time.` → `Please input hour (00-23):` → `[RTC] Current time: 12:34:56` |
| CTC | `[CTC] Start calibrating...` → `[CTC] Calibration OK! IRC48M is trimmed.` |
| Clock-out | `/=========== Clock output Demo ===========/` → `CK_OUT0: IRC16M, CK_OUT1: system clock/5`（按键切换） |
| PMU 睡眠 | `[Sleep] Demo started.` → `[Sleep] Sleeping now. Send any data from USART to wake up.` → `>>> [Wakeup] I am awake! <<<` |
| EXMC（历史 Harness 归档） | `---- [SDRAM] ----` / `[SDRAM-SLEEP] PASS` / `NAND ID: 0xC8 0xF1 0x80 0x1D` / `ALL TESTS COMPLETED SUCCESSFULLY!` |

---

## 6. 目录结构

```
GD32F470IKH6/                        # 仓库根目录
├── .gitignore                      # 忽略 Objects/、Listings/、*.o/*.d/*.hex/*.map 等构建产物
├── LICENSE                         # 用户代码 MIT（Copyright (c) 2026 琪瑶luo）
├── README.md                       # 本文档（默认 main = TRNG 测试）
├── RELEASE_NOTES.md                # v0.1.0 Release 正文（归档）
├── relnotes_v010.md                # v0.1.0 详细发布说明（归档）
└── GD32F470IKH6/                   # 源码根目录（与仓库根目录同名）
    ├── Project/                    # F470IKH6_demo.uvprojx / .uvoptx，构建产物 Objects/、Listings/
    ├── User/                       # main.c/.h、systick、gd32f4xx_it.c/.h、gd32f4xx_libopt.h、bsp_pin_def.h
    ├── BSP/                        # bsp_led/key/usart/adc/dac/i2c/spi/i2s/exmc +
    │                               # sdio/can/clockout/ctc/pmu/rtc/timer/tli/ipa/sccb/dci/trng（共 21 组）
    ├── App/                        # app_led/key/usart/adc/dac/i2c_eeprom +
    │                               # sdcard/can/clockout/ctc/sleep/rtc/breath/lcd/camera（共 15 组）
    ├── Components/
    │   ├── drivers/                # drv_at24c02 / drv_storage / drv_gd25qxx / drv_wav /
    │   │                           # drv_i2s_player / drv_sdcard / wave_data
    │   │   └── OV2640/             # dci_ov2640.c/.h、dci_ov2640_init_table.h
    │   └── images/                 # picture.c/.h + image1~12.h（LCD 显示素材）
    ├── OSAL/                       # 预留空目录（系统抽象层）
    ├── Services/                   # 预留空目录（服务层）
    ├── Firmware/                   # CMSIS + GD32F4xx 标准外设库（31 个 .c 参与编译）+ USB 库源码（未编译）
    ├── Doc/                        # 评估板原理图 / 用户指南 PDF、readme.txt（例程文档，UTF-8）
    ├── tools/                      # gen_wave.py（WAV 测试数据生成）
    └── .vscode/                    # Keil Assistant / IntelliSense 配置与日志
```

`.workbuddy/` 是本地助手的工作记录目录，与固件无关；`*.bak_*` 为调试备份，不参与编译。

---

## 7. 当前状态与已知限制

- **默认 `main.c` 只跑 TRNG 测试**（commit `9544e11`）：`BSP_USART_MODE_IRQ` + `bsp_trng_init()` + `app_usart_task()`；**未调用 `systick_config()`、`bsp_led_init()`、`bsp_key_init()`**，所以上电不点灯、按键无响应，这是当前代码的真实行为，不是故障。
- v0.1.0 的**全外设串联自检 Harness（`RUN_xx` / `AUTO_RUN_ALL` / `Test_xx()`）已从默认 `main.c` 移除**，仅在 `Doc/readme.txt` 第三章作为历史代码保留。要跑八项自检需从文档复制回 `main.c`。
- 2.2 表中所有模块都**已编译进固件但默认不调用**；切换例程靠替换 `main()`，没有编译开关。
- `BSP_USART_COMn = 1`，只有 `BSP_USART_COM0` 可用；传其它值会静默不初始化。
- `app_usart_task()` 用 256 字节静态行缓冲按 `\r`/`\n` 断帧，**超长行会被截断到 255 字节**；子串匹配用 `strstr()`，因此 `XXTRNGXX` 也会触发随机数；`BAUD9600` 优先级低于 `TRNG`。
- TRNG 例程发 `BAUD9600` 后 MCU 永久切到 9600（直到复位），终端不同步会看到乱码。
- `app_sleep_test()` 内部**包含死循环且进睡前关闭 SysTick 中断**，只能单独作为一个例程跑，不能和其它任务拼在同一 `main()` 里。
- `app_camera` / `app_lcd` 依赖片外 SDRAM 作图层/帧缓冲与 TLI 时序，`app_camera.c` 有多个 `*.bak_before_*` 备份，说明仍在调；`app_lcd` 的 12 张 `image*.h` 素材占用较大 Flash/ROM。
- 按键默认 `KEY_TRIG_FALLING_PULLUP`（上拉、按下为低、下降沿触发）；改成 `KEY_TRIG_RISING_FLOATING` 需把按键改成按下接 3.3V 的浮空接法。
- `app_key_polling_task()` 用 `while(...==0)` 死等松手，长按会阻塞主循环；`app_led_flow_task()` 阻塞约 3 秒。
- 双 ADC 需 PC3/PC5 接真实信号；`app_adc_task_dual_dma()` 打印 32 位同步寄存器原始值，需按手册拆两路 16 位数据。
- DAC 为 12 位右对齐（0~VDDA ≈ 3.3V），`app_dac_scan_task()` 需主循环不断调用。
- EXMC SDRAM/NAND 引脚由 `BSP/bsp_exmc.c` 按评估板原理图固定配置，换板需改该文件；`drv_storage_run()` 不负责引脚初始化。
- **编码**：部分源码中文注释为 **GBK**；`Doc/readme.txt`、`README.md`、`RELEASE_NOTES.md`、`relnotes_v010.md` 为 **UTF-8**。用错编码打开会乱码，不影响编译。
- `Firmware/` 下 CMSIS、标准外设库、USB 库版权归 GigaDevice Semiconductor Inc.，遵循其文件头 BSD-3-Clause；本仓库自有 `User/`、`BSP/`、`App/`、`Components/` 代码按根目录 `LICENSE` 的 MIT 许可使用。

---

## 8. 如何复核本 README（一致性依据）

| 声明 | 依据文件 |
| --- | --- |
| 默认 `main.c` = TRNG 测试（IRQ 串口 + `bsp_trng_init` + `app_usart_task`）、注释掉的 SD 卡 main | `User/main.c` |
| TRNG 初始化/重试/失败打印、32 位与区间随机数 | `BSP/bsp_trng.c/.h` |
| 按行断帧、`TRNG`/`BAUD9600` 指令、回显与 LED1 翻转 | `App/app_usart.c` |
| 串口两模式、256B 缓冲、`set_baudrate`/`send_dma`/`get_rx_data` | `BSP/bsp_usart.c/.h` |
| 全部 `bsp_` / `app_` / `drv_` 头汇总、`drv_i2s_player.h` 被注释 | `User/gd32f4xx_libopt.h` |
| 基础引脚 + SDIO / CAN0 / TLI / SCCB / DCI / OV2640 地址 / SDIO DMA 定义 | `User/bsp_pin_def.h` |
| EXTI0 / EXTI10_15 / DMA1_Channel7 / SPI1 / SDIO 中断服务位置 | `User/gd32f4xx_it.c` |
| 编译进工程的 `.c` 清单（21 组 BSP、15 组 App、31 个标准库 `.c`、dci_ov2640.c、picture.c） | `Project/F470IKH6_demo.uvprojx` |
| 各新增模块的公共 API 与打印字符串 | `BSP/*.h`、`App/*.c/.h`、`Components/drivers/*.c/.h` |
| 29 例程框架、历史 Harness 完整代码、各例程 `main()` 模板 | `Doc/readme.txt` |
| 引脚分配、JP5 跳线、MAX3232/COM0、115200 参数 | `Doc/GD32470I-EVAL评估板用户指南_V2.0.pdf` |
| 器件、DFP 版本、AC6/C99/-O2、USB 库未编译 | `Project/F470IKH6_demo.uvprojx` |
| clone 地址 | `git remote -v`（`https://github.com/you-678/GD32F470IKH6.git`） |
