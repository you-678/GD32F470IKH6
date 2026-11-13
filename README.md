# GD32F470IKH6 裸机学习工程

基于 **GD32470I-EVAL 评估板（主控 GD32F470IKH6，Cortex-M4）** 的 Keil MDK-ARM 单工程，把官方分散的入门 Demo 抽成 `BSP / App / User` 分层，并引入 `Components/drivers` 组件层与 `bsp_pin_def.h` 集中引脚定义，方便在一个工程里切换、对比 GPIO 轮询与中断、串口轮询与中断/DMA、单/双 ADC 同步、DAC 输出、I2C/SPI 读写、EXMC 外扩 SDRAM/NAND、I2S 音频播放等写法。**换板子只改 `bsp_pin_def.h` 一个文件即可**。

> **当前状态（以 `main.c` 为准，commit `584d883` 之后）**：工程默认运行的是 **「GD32F470 Full Test」综合自检**——上电初始化 USART0 打印进度、点亮三颗 LED，先跑**阶段 1 按键轮询测试（5 秒，按 PA0/PC13/PB14 翻转对应 LED）**，再跑**阶段 2 EXMC 存储测试**（`drv_storage_run()` 依次测 SDRAM → SDRAM 休眠 → NAND，每项约 1 秒），打印 `ALL TESTS COMPLETED SUCCESSFULLY!` 后切到**按键 EXTI 中断模式**进入空闲死循环（此后按任意键翻转对应 LED）。LED / KEY / USART / ADC / DAC / I2C / SPI / I2S / EXMC 的例程函数都在源码里，需按第 4.2 节手动改 `main.c` 才能跑其他例程。本 README 描述当前仓库已提交的工作区版本。

> **文档基准（2026-11-13）**：本文行为描述逐条对照当前工作区源码与工程配置核实，依据为 `User/main.c`、`User/bsp_pin_def.h`、`App/*.c`、`BSP/*.c/.h`、`Components/drivers/*.c/.h`、中断文件 `User/gd32f4xx_it.c`、`User/gd32f4xx_libopt.h`、`Project/F470IKH6_demo.uvprojx`。凡「只是函数、未被默认 main 调用」的能力都在第 2、7 节显式标注，**不计入默认功能**；复核方法见第 8 节。

---

## 1. 项目解决什么问题

| 痛点 | 本仓库的做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，想对比流水灯 / 按键轮询 / EXTI 中断 / 串口打印 / 单双 ADC / DAC / I2C / SPI / EXMC / I2S 时要反复建工程、切工程 | 把各外设抽成 `BSP/` 板级驱动，把例程逻辑抽成 `App/` 任务函数；**只改 `User/main.c` 就能切换例程**，`BSP/`、`App/`、`Components/` 的 `.c` 已全部在工程文件组内 |
| 裸机调试没有日志，只能靠点灯猜状态 | 实现 `printf` 重定向到 USART0（PA9/PA10，115200-8-N-1），可直接用串口助手看输出（含浮点，见第 3.4 节） |
| 换板子要改一堆散落的引脚宏 | 所有引脚/时钟宏集中在 `User/bsp_pin_def.h`，驱动层与业务层都从它取，换 MCU/板卡只改一处 |
| 学习过程中「代码写了、现象没记」 | `Doc/readme.txt` 记录分层架构、29 例程总览与多份完整 `main()` 调用示例，作为历史参考 |

**范围边界（避免误读）**：

- 本仓库**不是** 29 个官方例程的移植合集。`Doc/readme.txt` 里 01–29 是例程框架，其中 01/03/06/09/10/12/13/14–16 等有对应 `main()` 示例，其余多为待补/框架标题。
- 本仓库**不是**固件产品或飞控代码，不含 RTOS、协议栈、算法。`Firmware/GD32F4xx_usb_library/` 虽带官方 USB Device/Host 库源码，但 `uvprojx` 中引用数为 0，**未加入编译、无例程调用**。目录里的 `OSAL/`、`Services/`、`Components/protocols`、`Components/third_party` 是**预留的空分层目录**，当前只有 `Components/drivers/` 下的组件落地。
- 验证环境：µVision V5.40.0.0 / MDK-ARM Plus 5.40.0.3 / **ARMCLANG V6.19（AC6，C99，-O2）** / `GigaDevice.GD32F4xx_DFP 3.2.0`。本仓库通过 `.gitignore` 忽略所有构建产物（`Objects/`、`Listings/`、`*.o`、`*.hex` 等），**clone 后无固件、无构建日志，须本地 Keil 编译**；具体 Code/RO/RW/ZI 体积以本地 Build Output 为准。

---

## 2. 主要功能

| 模块 | 真实实现 | 关键文件 | 状态 |
| --- | --- | --- | --- |
| LED BSP+App | 3 路 GPIO 输出：LED1=PE2、LED2=PE3、LED3=PF10（置高点亮、置低熄灭，推挽 50MHz）；`init/on/off/toggle`，`app_led_all_on/off/toggle()`、`app_led_flash(n)`（亮 500ms→灭 500ms，循环 n 次）、`app_led_flow_task()`（LED1→LED2→LED3 各亮 1 秒，阻塞约 3 秒） | `BSP/bsp_led.c/.h`、`App/app_led.c` | 可用（默认 main 用作状态指示） |
| KEY BSP+App | 3 按键：Wakeup=PA0、Tamper=PC13、User=PB14；`KEY_MODE_GPIO` 输入轮询 / `KEY_MODE_EXTI` 中断；支持**注册回调** `bsp_key_register_callback()`；新增 `bsp_key_set_trigger()` 可切换触发方式，**默认 `KEY_TRIG_FALLING_PULLUP`（上拉、按下为低、下降沿触发 → 按下即回调）** | `BSP/bsp_key.c/.h`、`App/app_key.c` | 可用（**默认 main 阶段 1 轮询、阶段 2 切 EXTI 中断**） |
| USART0 BSP+App | PA9(TX)/PA10(RX) AF7，8/1/无校验/无流控，波特率参数传入；`fputc` 重定向 `printf`；`PRINTF` / `IRQ` 两模式；`bsp_usart_set_baudrate()`（运行中改波特率）、`bsp_usart_send_dma()`（DMA 发送）、`bsp_usart_get_rx_data()`（取接收缓冲）；256 字节接收缓冲在 `bsp_usart.c` 内 | `BSP/bsp_usart.c/.h`、`App/app_usart.c` | PRINTF 可用（**默认 main 用其打印进度**）；IRQ 回显可用（需切到 IRQ 模式调用 `app_usart_task()`） |
| ADC BSP+App | `bsp_adc_init(mode)` 三模式：**内部通道**（CH16 温度/CH17 Vref/CH18 Vbat，软件触发）、**双 ADC 跟随 DMA**（CH13=PC3/CH15=PC5，TIMER1 硬件触发，DMA1_CH0 搬运）、**双 ADC 并行 DMA**（同上通道，并行同步）；提供 `trigger()` 与读内部 `idata0/1/2`、DMA `data0/1` 的接口 | `BSP/bsp_adc.c/.h`、`App/app_adc.c` | 可用（默认 main 未调用） |
| DAC BSP+App | PA4，12 位右对齐；`bsp_dac_init()`（关触发、关波形、使能输出缓冲、初始 0V）、`bsp_dac_set_value()`（0~4095 → 0~VDDA）；`app_dac_scan_task()` 每 100ms 阶梯 +100（约 +0.08V），到顶转降、到底转升，非阻塞需循环调用 | `BSP/bsp_dac.c/.h`、`App/app_dac.c` | 可用（默认 main 未调用） |
| I2C BSP | I2C0（SCL=PB6 / SDA=PB7，AF4，100kHz，开漏上拉）；**硬件 I2C**（`bsp_i2c_init(BSP_I2C_MODE_HW)`）+ **软件 I2C**（GPIO 模拟时序，两套都带超时与 `printf` 报错）对外统一接口 `bsp_i2c_write_bytes()` / `bsp_i2c_read_bytes()` / `bsp_i2c_wait_standby()` | `BSP/bsp_i2c.c/.h` | 可用（默认 main 未调用） |
| AT24C02 组件驱动 | 芯片无关驱动：探测在线 `at24c02_init()`、单字节读写、按页写 `at24c02_write_buffer()`（内部自动分页）、连续读 `at24c02_read_buffer()`、整片写读一致性 `at24c02_test()`（256 字节全测） | `Components/drivers/drv_at24c02.c/.h` | 可用（被 `app_i2c_eeprom` 调用） |
| I2C-EEPROM App | `app_i2c_eeprom_init(mode)`：初始化 LED（灭）、初始化 I2C、探测 AT24C02、跑一致性测试，通过置 `s_ok`；`app_i2c_eeprom_task()`：通过则三灯循环单闪，失败则三灯常亮 | `App/app_i2c_eeprom.c` | 可用（默认 main 未调用，可手动切） |
| EXMC BSP | 外部存储器控制器：**SDRAM**（32MB @ `0xC0000000`，含 `bsp_sdram_write/read`、`bsp_sdram_enter/exit_self_refresh`）+ **NAND**（@ `0x70000000`，含 `bsp_nand_read_id/write/read/erase_block/reset`）；`bsp_exmc_init_mode()` 按模式初始化，`bsp_exmc_restore_clock()` 恢复时钟 | `BSP/bsp_exmc.c/.h` | 可用（**默认 main 阶段 2 通过 `drv_storage_run()` 调用**） |
| Storage 组件驱动 | `drv_storage_run(mode)` 统一跑存储模式：`STORAGE_MODE_SDRAM` / `STORAGE_MODE_SDRAM_SLEEP` / `STORAGE_MODE_NAND`；`drv_storage_mode_name()` 返回模式名 | `Components/drivers/drv_storage.c/.h` | 可用（**默认 main 阶段 2 调用**） |
| SPI BSP | `BSP_SPI_BUS_FLASH`（SPI5，PG13/12/14/10/11 + CS PI8，AF5，预分频 PSC_8 → 15MHz）、`BSP_SPI_BUS_LCD`；**硬件 SPI**（`bsp_spi_init_mode(HW)`）+ **软件 SPI**（GPIO 模拟）；统一 `bsp_spi_transfer/_buf`、`bsp_spi_cs_low/high`、Quad 模式 `bsp_spi_quad_enter_read/write/exit` | `BSP/bsp_spi.c/.h` | 可用（默认 main 未调用） |
| GD25Qxx Flash 组件驱动 | 芯片无关 SPI Flash 驱动：`gd25qxx_bind/probe/read_id`、状态读写、`write_enable`、`wait_busy`、扇区/块/整片擦除、页读写、`gd25qxx_quad_enable/read/write`（Quad 模式） | `Components/drivers/drv_gd25qxx.c/.h` | 可用（默认 main 未调用，可手动切） |
| I2S BSP | `BSP_I2S_BUS_AUDIO`（I2S1=SPI1：MCK=PA6 AF6、BCK=PI1 AF5、WS=PI0 AF5、SD=PC1 AF7）；**硬件 I2S**（`bsp_i2s_init_mode(HW)`）+ **软件 I2S**；`bsp_i2s_start()`（带发送回调、`num_halfwords` 采样数）、`bsp_i2s_stop()`、`bsp_i2s_is_done()`、`bsp_i2s_irq_handler()` | `BSP/bsp_i2s.c/.h` | 可用（默认 main 未调用） |
| WAV / I2S Player 组件 | `drv_wav_parse/iter_init/iter_next` 解析与逐帧迭代 WAV；`drv_i2s_player_init/play_blocking` 通过 I2S 播放 WAV；`wave_data.c` 内含一段嵌入的测试 WAV（`wavetestdata` / `wavetestdata_len`） | `Components/drivers/drv_wav.c/.h`、`drv_i2s_player.c/.h`、`wave_data.c/.h` | 可用（默认 main 未调用，可手动切） |
| 系统 / 中断 | `systick_config()` 配 1kHz SysTick → `delay_1ms()`；`EXTI0`/`EXTI10_15` 中断服务调用 `bsp_key_irq_handler()` 分发按键；`USART0_IRQHandler` 下沉到 `BSP/bsp_usart.c`；`SPI1_IRQHandler` 下沉到 `BSP/bsp_i2s.c` | `User/systick.c`、`User/gd32f4xx_it.c`、`BSP/bsp_usart.c`、`BSP/bsp_i2s.c` | 可用 |
| 官方固件 | CMSIS（startup + `system_gd32f4xx.c`，25MHz HXTAL 经 PLL 配 200MHz 系统时钟）+ 整套 GD32F4xx 标准外设库（27 个 `.c`），全部加入工程编译 | `Firmware/CMSIS/`、`Firmware/GD32F4xx_standard_peripheral/` | 可用 |
| 引脚集中定义 | 所有 LED/KEY/USART/ADC/DAC/I2C/SPI/I2S/EXMC 的 GPIO 端口、引脚、时钟、AF、外设、中断号宏统一在此 | `User/bsp_pin_def.h` | 可用 |
| 工程与资料 | Keil 工程、VS Code / Keil Assistant 配置、评估板原理图与用户指南 PDF、29 例程文档、`tools/gen_wave.py`（WAV 数据生成） | `Project/`、`.vscode/`、`Doc/`、`tools/` | — |

**状态图例**：`可用` = 源码在且编译进固件；`默认 main 调用` = 当前 `main.c` 会跑；`默认 main 未调用` = 需按第 4.2 节手动改 `main.c`。

> 注：USART 的旧 `BSP_USART_MODE_DMA` 枚举与独立 `bsp_usart_dma_rx_init/tx_start` 已被移除；串口 DMA 现在以「`bsp_usart_send_dma()` 发送 + IRQ 接收 + `bsp_usart_get_rx_data()` 取数」的形式给出，回显链路是通的（见第 4.3 节）。

---

## 3. 安装方法

### 3.1 软件环境

| 项 | 要求 |
| --- | --- |
| 操作系统 | Windows（Keil µVision 工程，无 Makefile / CMake 构建链） |
| IDE / 编译器 | Keil MDK-ARM 5.40（本机 5.40.0.3），ARMCLANG V6.19（AC6）；工程已勾选 `uAC6`、`C99`、优化等级 `-O2` —— 其他 AC6 版本未验证 |
| 器件支持包 | `GigaDevice.GD32F4xx_DFP 3.2.0`；工程选定器件 `GD32F470IK`，IROM 3MB @ `0x08000000`，Flash 算法 `GD32F4xx_3MB.FLM` |
| 串口终端 | 任意支持 115200 / 8-N-1 / 无流控的串口助手 |
| 可选 | VS Code + Keil Assistant 插件（`.vscode/` 已带 IntelliSense 配置） |

### 3.2 硬件与接线

- **板卡**：GD32470I-EVAL（GD32F470IKH6），Mini USB 或 DC-005 提供 5V（来源：用户指南 V2.0 第 1 节）。**该板载有外部 SDRAM 与 NAND Flash**，默认例程的阶段 2（EXMC 存储测试）依赖它们。
- **下载**：工程调试适配器配置为 CMSIS-DAP「Any」/ SW-DP（`CMSIS_AGDI.dll`），未锁定具体型号；板载 GD-Link 或外置 J-Link / CMSIS-DAP 均可，需在 `Options for Target → Debug` 里选定实际调试器。
- **串口（关键）**：USART0 = PA9(TX) / PA10(RX)。评估板经 MAX3232 引出到 DB9 **COM0**，**必须用跳线帽把 JP5 短接 1-2 选择 USART0**（短接 2-3 是 USB_FS）。COM0 是 **RS232 电平**，请用 USB-RS232 串口线；如果用 USB-TTL，请直接接 PA9 / PA10 / GND 的 **TTL 电平**，不要接 DB9（来源：用户指南 V2.0 第 15 页原理图、第 30 页串口参数）。
- **SPI Flash（切换例程需要）**：SPI5 = SCK PG13 / MISO PG12 / MOSI PG14 / IO2 PG10 / IO3 PG11（均 AF5），CS=PI8（推挽）；接 GD25Qxx 系列 SPI Flash（如 GD25Q16/32/64/128），支持单线/Quad。
- **I2S 音频（切换例程需要）**：I2S1=SPI1 = MCK PA6(AF6) / BCK PI1(AF5) / WS PI0(AF5) / SD PC1(AF7)；外接 I2S DAC/功放或耳机 amp 听测试音。
- **I2C / EEPROM（切换例程需要）**：I2C0 = PB6(SCL) / PB7(SDA)，外部上拉到 3.3V，接 AT24C02（或兼容）器件，地址 `0xA0`，容量 256 字节、页大小 8 字节。
- **ADC 外部通道（切换例程需要）**：CH13 = PC3、CH15 = PC5（均 `GPIO_MODE_ANALOG`）。「内部通道模式」采集片内温度/Vref/Vbat，**无需外部接线**；「双 ADC 跟随/并行模式」需要把 PC3、PC5 接到待测信号。
- **DAC 输出（切换例程需要）**：PA4，接万用表/示波器测电压（0~3.3V 阶梯）。
- **EXMC（默认例程依赖）**：SDRAM @ `0xC0000000`（32MB）、NAND @ `0x70000000`，引脚由 `BSP/bsp_exmc.c` 按 GD32470I-EVAL 原理图配置。
- 引脚分配与代码完全一致（来源：用户指南 V2.0 表 2-1，第 8 页）：

| 功能 | 引脚 | 功能 | 引脚 |
| --- | --- | --- | --- |
| LED1 | PE2 | Wakeup 键 | PA0 |
| LED2 | PE3 | Tamper 键 | PC13 |
| LED3 | PF10 | User 键 | PB14 |
| USART0_TX | PA9 | USART0_RX | PA10 |
| ADC_CH13 | PC3 | ADC_CH15 | PC5 |
| DAC_OUT | PA4 | I2C_SCL | PB6 |
| I2C_SDA | PB7 | SPI_FLASH_SCK | PG13 |
| SPI_FLASH_MISO | PG12 | SPI_FLASH_MOSI | PG14 |
| SPI_FLASH_IO2 | PG10 | SPI_FLASH_IO3 | PG11 |
| SPI_FLASH_CS | PI8 | I2S_MCK | PA6 |
| I2S_BCK | PI1 | I2S_WS | PI0 |
| I2S_SD | PC1 | | |

### 3.3 获取与编译

```bash
git clone https://github.com/you-678/GD32F470IKH6.git
cd GD32F470IKH6
```

1. 用 µVision 打开 **`GD32F470IKH6/Project/F470IKH6_demo.uvprojx`**（仓库根目录和源码目录同名，是两层 `GD32F470IKH6/`，别只进第一层；Target：`GD32F470IKH6_demo`）。
2. `Project → Build Target`（F7）。产物在 `GD32F470IKH6/Project/Objects/`：`F470IKH6_demo.axf`、`F470IKH6_demo.hex`、`F470IKH6_demo.build_log.htm`（构建日志）、`F470IKH6_demo.htm`（静态调用图 / 栈用量）。**本工程未生成 `.map` 文件**。
3. `Download`（F8，或把 `.hex` 烧进 Flash）后复位运行。

> `.gitignore` 忽略了 `Objects/`、`Listings/`、`*.o`、`*.d`、`*.hex`、`*.map`，**clone 后仓库里没有固件，也没有构建日志，必须自行编译**。

### 3.4 常见报错与排错

| 现象 | 处理 |
| --- | --- |
| `#error "Please select the target GD32F4xx device in gd32f4xx.h file"` | `Firmware/CMSIS/GD/GD32F4xx/Include/gd32f4xx.h` 里的器件宏是注释状态，本机由 DFP 环境注入；换环境后报此错，在 `Options for Target → C/C++ (AC6) → Define` 添加 `GD32F470` |
| 串口无输出 | 依次查：JP5 是否短接 1-2、串口线是否接 COM0、终端是否 115200-8-N-1、是否误把 RS232 口接到 USB-TTL；默认 `main.c` 上电即有串口打印（见第 5 节），与按键无关 |
| 默认例程阶段 1 按键无效 | 确认按键为上拉接法（默认 `KEY_TRIG_FALLING_PULLUP`，按下为低电平触发）；若改过 `bsp_key_set_trigger()` 为 `RISING_FLOATING`，则需改成按下接 3.3V |
| 默认例程阶段 2 存储测试报错 | GD32470I-EVAL 板载 SDRAM/NAND 正常时通过；若在其它板卡或 SDRAM/NAND 未焊接，存储读写会失败（打印错误后继续），属预期 |
| 温度/电压打印为 `0.000` | 仅当改用 **Microlib** 时才需要；请在 `Options for Target → C/C++ (AC6)` 勾选「Use float with printf」再编译。本工程用 ARMCLANG 标准库（非 Microlib），`%f` 默认可用 |
| 双 ADC 模式读数恒为 0 / 乱跳 | 内部通道模式无需接线；双 ADC 模式需 PC3、PC5 接真实模拟信号，悬空会读到噪声 |
| DAC 输出电压不变 | 切换例程未把 PA4 接到测量端；`app_dac_scan_task()` 需在主循环不断调用（每次含 10ms 延时）才产生阶梯波 |
| SPI Flash 探测失败 | 检查 SPI5 引脚接线、CS=PI8 电平、Flash 型号；Quad 模式需 `gd25qxx_quad_enable()` 且器件支持 QE 位 |
| I2S 无声音 | 确认 I2S1 四线（MCK/BCK/WS/SD）接线与外接 DAC 匹配；`drv_i2s_player_play_blocking()` 会按 `wavetestdata` 的采样率/位数配置 I2S |
| 中文注释乱码 | 部分源码与 `Doc/readme.txt` 是 GBK 编码，用 GBK / GB18030 打开即可，不影响编译 |
| VS Code Keil Assistant 提示「发现无效的文件组」 | 插件解析 `OSAL/`/`Services/` 等空目录告警，不影响 µVision 编译 |

---

## 4. 使用方法

### 4.1 跑通默认例程（GD32F470 Full Test：按键轮询 → EXMC 存储 → EXTI 中断）

1. JP5 短接 1-2，串口线接 COM0，打开串口助手：**115200 / 8 数据位 / 1 停止位 / 无校验 / 无流控**。
2. 使用 GD32470I-EVAL 板（板载 SDRAM + NAND）；接好 5V 供电。
3. 编译（F7）→ 下载（F8）→ 复位。
4. 上电观察串口与板载 LED：
   - 先打印 `GD32F470 Full Test Boot` 横幅，三灯点亮；
   - **阶段 1（约 5 秒）**：打印 `[STAGE 1] POLLING TEST`，期间**按 PA0/PC13/PB14 任一键，对应 LED1/LED2/LED3 翻转**并打印 `Pressed`；
   - **阶段 2（约 3 秒）**：打印 `[STAGE 2] EXMC STORAGE TEST`，依次跑 SDRAM → SDRAM 休眠 → NAND（每项约 1 秒），打印各模式结果；
   - 打印 `ALL TESTS COMPLETED SUCCESSFULLY!` 后进入 EXTI 中断模式，**此后按任意键翻转对应 LED**，主循环空闲。

### 4.2 切换例程

把下面片段替换 `User/main.c` 的 `main()` 主体即可（需要的外设 `init` 已写在片段里）。`App/`、`BSP/`、`Components/` 的 `.c` 无需增删。改完重新 Build + Download。

```c
#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"

int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    bsp_led_init(LED1); bsp_led_init(LED2); bsp_led_init(LED3);

    /* ① 流水灯：初始化三颗 LED，循环跑流水 */
    // while(1) { app_led_flow_task(); }

    /* ② 按键轮询：初始化三键为轮询模式，循环调用轮询任务 */
    // app_key_init_polling();
    // while(1) { app_key_polling_task(); }

    /* ③ 按键中断：初始化三键为中断模式并注册回调，主循环留空 */
    // app_key_init_exti();
    // while(1) { }

    /* ④ 串口 IRQ 回显 + 命令：初始化为 IRQ 模式，循环调用 USART 任务 */
    // bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);
    // while(1) { app_usart_task(); }

    /* ⑤ ADC 三模式自动测试 */
    // bsp_adc_init(BSP_ADC_MODE_INTERNAL);
    // while(1) { /* 见 Doc/readme.txt 09 例程：内部/跟随DMA/并行DMA 三段循环 */ }

    /* ⑥ DAC 阶梯波：初始化 DAC，循环扫描 */
    // bsp_dac_init();
    // while(1) { app_dac_scan_task(); delay_1ms(10); }

    /* ⑦ I2C-EEPROM（硬件/软件 I2C 可选） */
    // app_i2c_eeprom_init(BSP_I2C_MODE_HW);
    // while(1) { app_i2c_eeprom_task(); }

    /* ⑧ SPI Flash（GD25Qxx）测试 */
    // /* 见 Doc/readme.txt 中 GD25Qxx 双模自检片段 */

    /* ⑨ I2S 音频播放（WAV 测试数据） */
    // /* 见 Doc/readme.txt 中 I2S Player 自检片段 */

    /* ⑩ 默认 Full Test（取消下面注释即恢复出厂默认例程） */
    app_key_init_polling();
    for (uint32_t i = 0; i < 5000; i++) { app_key_polling_task(); delay_1ms(1); }
    for (int m = 0; m < STORAGE_MODE_MAX; m++) { drv_storage_run((storage_mode_t)m); delay_1ms(1000); }
    app_key_init_exti();
    while (1) { delay_1ms(1000); }
}
```

更多完整 `main()` 示例（含 01 流水灯、03 EXTI、06 USART、09 ADC、10 DAC、GD25Qxx Flash、I2S Player、默认 Full Test）见 `Doc/readme.txt`。

### 4.3 串口两种模式的真实状态

`bsp_usart_init()` 第 2 个参数：

| 模式 | 代码做了什么 | 能否当功能用 |
| --- | --- | --- |
| `BSP_USART_MODE_PRINTF` | 仅轮询发送，`printf` 重定向到 USART0；接收始终使能但无中断，需自行读寄存器 | ✅ **当前默认 main 使用**，用于打印 Full Test 进度，**只发不收** |
| `BSP_USART_MODE_IRQ` | 使能 RBNE 中断 + NVIC；`USART0_IRQHandler`（在 `bsp_usart.c`）把字节存入 256 字节 `rx_buffer`，每收到一字节置 `rx_complete=1`；`app_usart_task()` 取数后 `printf` 回显并 `bsp_usart_send_dma()` 用 DMA 发回 | ✅ **回显链路已通**：发任意字符串回显 `[IRQ RX] Got: <原串>`；发 `BAUD9600` 切到 9600 波特率 |

> **串口号不可随便改**：`usart_typedef_enum` 只有 `BSP_USART_COM0`（其后 `BSP_USART_COMn = 1` 为计数），`bsp_usart_init()` 首行 `if (usart_num >= BSP_USART_COMn) return;`。**传其他值会静默不初始化**，只有 COM0 可用；要扩展需补齐 `bsp_usart.c` 里的查表数组。

### 4.4 常用接口速查

```c
/* LED (BSP/bsp_led.h) */
void bsp_led_init(led_typedef_enum lednum);            /* LED1/LED2/LED3 */
void bsp_led_on/off/toggle(led_typedef_enum lednum);

/* KEY (BSP/bsp_key.h) */
void bsp_key_init(key_typedef_enum k, keymode_typedef_enum mode);
void bsp_key_set_trigger(key_trigger_typedef_enum trigger); /* FALLING_PULLUP(默认) / RISING_FLOATING */
uint8_t bsp_key_state_get(key_typedef_enum k);         /* 0 = 按下 */
void bsp_key_register_callback(key_typedef_enum k, key_isr_callback_t cb);
void bsp_key_irq_handler(uint32_t exti_line);          /* 供 gd32f4xx_it.c 调用 */

/* USART0 (BSP/bsp_usart.h) */
void bsp_usart_init(usart_typedef_enum com, usartmode_typedef_enum mode, uint32_t baudrate);
void bsp_usart_set_baudrate(usart_typedef_enum com, uint32_t baudrate);
void bsp_usart_send_dma(usart_typedef_enum com, uint8_t *data, uint16_t len);
uint8_t bsp_usart_get_rx_data(usart_typedef_enum com, uint8_t *data, uint16_t *len);

/* ADC (BSP/bsp_adc.h) */
void bsp_adc_init(bsp_adc_mode_enum mode);             /* INTERNAL / DUAL_DMA_FOLLOW_UP / DUAL_DMA_PARALLEL */
void bsp_adc_trigger(void);
uint16_t bsp_adc_get_idata0/1/2(void);                /* 内部通道：温度/参考电压/电池电压 */
uint32_t bsp_adc_get_dma_data0/1(void);               /* 双 ADC DMA 原始值 */

/* DAC (BSP/bsp_dac.h) */
void bsp_dac_init(void);
void bsp_dac_set_value(uint16_t value);               /* 0~4095 -> 0~VDDA(约3.3V) */

/* I2C (BSP/bsp_i2c.h) */
void bsp_i2c_init(bsp_i2c_mode_enum mode);            /* BSP_I2C_MODE_HW / BSP_I2C_MODE_SW */
uint8_t bsp_i2c_write_bytes(uint8_t dev, uint8_t reg, uint8_t *p, uint16_t len);
uint8_t bsp_i2c_read_bytes(uint8_t dev, uint8_t reg, uint8_t *p, uint16_t len);
void bsp_i2c_wait_standby(uint8_t dev);

/* EXMC (BSP/bsp_exmc.h) */
void bsp_exmc_init_mode(bsp_exmc_mode_t mode);        /* BSP_EXMC_MODE_SDRAM / BSP_EXMC_MODE_NAND */
void bsp_sdram_write/read(uint8_t *buf, uint32_t addr, uint32_t len);
void bsp_sdram_enter/exit_self_refresh(void);
void bsp_nand_read_id(bsp_nand_id_t *id);
int  bsp_nand_write/read(uint32_t memaddr, uint8_t *buf, uint16_t len);
int  bsp_nand_erase_block(uint32_t block);

/* Storage 组件 (Components/drivers/drv_storage.h) */
const char *drv_storage_mode_name(storage_mode_t mode);
int         drv_storage_run(storage_mode_t mode);     /* SDRAM / SDRAM_SLEEP / NAND */

/* SPI (BSP/bsp_spi.h) */
void bsp_spi_init_mode(bsp_spi_bus_t bus, bsp_spi_mode_t mode); /* BUS_FLASH/LCD, MODE_HW/SW */
uint8_t bsp_spi_transfer(bsp_spi_bus_t bus, uint8_t tx);
void    bsp_spi_cs_low/high(bsp_spi_bus_t bus);

/* GD25Qxx Flash 组件 (Components/drivers/drv_gd25qxx.h) */
int gd25qxx_probe(gd25qxx_dev_t *dev);
int gd25qxx_read/write(gd25qxx_dev_t *dev, uint8_t *buf, uint32_t addr, uint32_t len);
int gd25qxx_quad_read/write(gd25qxx_dev_t *dev, uint8_t *buf, uint32_t addr, uint32_t len);

/* I2S (BSP/bsp_i2s.h) */
void bsp_i2s_init_mode(bsp_i2s_bus_t bus, bsp_i2s_mode_t mode);
void bsp_i2s_start(bsp_i2s_bus_t bus, uint32_t sr_hz, bsp_i2s_tx_cb_t cb, void *ctx, uint32_t num_halfwords);
void bsp_i2s_stop(bsp_i2s_bus_t bus);

/* WAV / I2S Player 组件 */
int  drv_wav_parse(const uint8_t *data, uint32_t len, drv_wav_info_t *info);
void drv_i2s_player_init(drv_i2s_player_t *p, bsp_i2s_bus_t bus);
int  drv_i2s_player_play_blocking(drv_i2s_player_t *p, const uint8_t *wav, uint32_t len);

/* AT24C02 组件 (Components/drivers/drv_at24c02.h) */
uint8_t at24c02_init(void);
uint8_t at24c02_write_byte(uint8_t addr, uint8_t data);
uint8_t at24c02_read_byte(uint8_t addr, uint8_t *data);
uint8_t at24c02_write_buffer(uint8_t addr, uint8_t *buf, uint16_t len);
uint8_t at24c02_read_buffer(uint8_t addr, uint8_t *buf, uint16_t len);
uint8_t at24c02_test(void);

/* App 任务 (App/*.h) */
void app_led_all_on/off/toggle(void);
void app_led_flash(uint8_t times);  void app_led_flow_task(void);
void app_key_init_polling(void);    void app_key_init_exti(void);  void app_key_polling_task(void);
void app_usart_task(void);
void app_adc_task_internal(void);   void app_adc_task_dual_dma(void);
void app_dac_scan_task(void);
void app_i2c_eeprom_init(bsp_i2c_mode_enum mode);  void app_i2c_eeprom_task(void);

/* 系统 */
void systick_config(void);     void delay_1ms(uint32_t ms);

/* 引脚集中定义 (User/bsp_pin_def.h)：LED/KEY/USART/ADC/DAC/I2C/SPI/I2S/EXMC 全在此 */
```

这些声明由 `User/gd32f4xx_libopt.h` 统一汇总（含标准外设库头、`stdio.h`/`string.h`、`systick.h`、全部 `BSP/`、`App/`、`Components/` 头与 `bsp_pin_def.h`），`gd32f4xx.h` 会自动包含它，因此各 `.c` 只要 `#include "gd32f4xx.h"` 就能拿到全部声明。

---

## 5. 输入输出示例

### 5.1 输入

| 输入 | 形式 | 代码路径 / 说明 |
| --- | --- | --- |
| 上电 / 复位 | 硬件 | `systick_config()` → `bsp_usart_init(COM0, PRINTF, 115200)` → `app_key_init_polling()` → 轮询循环 → `drv_storage_run()` 三模式 → `app_key_init_exti()` → 空闲 |
| 代码层参数 | `User/main.c` | USART 串口号 COM0、模式 PRINTF、波特率 115200；存储测试 `STORAGE_MODE_MAX=3` |
| 按键（阶段 1） | 低电平按下 / 上拉 | `app_key_polling_task()` 20ms 消抖，按下翻转对应 LED 并打印 |
| 按键（阶段 2 之后） | 低电平按下 / 上拉 | EXTI 下降沿（`KEY_TRIG_FALLING_PULLUP`）触发回调，翻转对应 LED |
| （切换例程后）串口助手下发 | 文本，以 `\r\n` 结尾最佳 | IRQ 模式下由 `app_usart_task()` 取数；含 `BAUD9600` 触发降速，否则回显 |
| （切换例程后）PC3 / PC5 模拟信号 | 电压 | 双 ADC 模式由 TIMER1 触发采样，DMA 搬运 |
| （切换例程后）PA4 测量端 | 万用表/示波器 | DAC 阶梯波输出 |
| （切换例程后）SPI Flash / I2S 外设 | 硬件 | 见 `Doc/readme.txt` 对应自检片段 |
| 板载 SDRAM / NAND | 外部存储 | 默认例程阶段 2 由 `drv_storage_run()` 读写验证 |

### 5.2 输出

**① 默认 `main.c` 上电后串口（综合自检，GD32470I-EVAL 板）**

```
========================================
GD32F470 Full Test Boot
========================================

[STAGE 1] POLLING TEST (5 seconds)...
Press PA0/PC13/PB14 to toggle LEDs.
[Polling] PA0 Pressed! LED1 Toggle.
[Polling] PC13 Pressed! LED2 Toggle.

[STAGE 2] EXMC STORAGE TEST...
（依次打印 SDRAM / SDRAM Sleep / NAND 各模式测试结果）

========================================
ALL TESTS COMPLETED SUCCESSFULLY!
You can still press keys to toggle LEDs.
========================================
```

> 阶段 1 约 5 秒，期间按 PA0/PC13/PB14 任一对 LED1/LED2/LED3 翻转并打印 `Pressed`；阶段 2 约 3 秒跑完三种存储模式；之后进入 EXTI 中断模式，按任意键即可翻转对应 LED。**若板载无 SDRAM/NAND 或焊接异常，存储测试会打印错误但继续**，不影响进入中断模式。

**② 切换为「串口 IRQ 回显」例程后**（`app_usart_task()` 在循环中）：

```
[IRQ RX] Got: hello
（同时 DMA 把 "hello" 原样发回串口助手）

Changing baudrate to 9600...
[Success] Baudrate is now 9600 bps.
（发送 "BAUD9600" 后，后续通信需把终端切到 9600）
```

**③ 切换为「按键 EXTI 中断」例程后**：默认触发为**下降沿（按下即触发）**，按下 PA0/PC13/PB14 立即翻转 LED1/LED2/LED3 并打印 `Pressed`（注意与旧版「松手触发」不同）。

**④ 切换为「DAC 阶梯波」例程后**：PA4 输出 0V → 3.3V → 0V 循环扫描，万用表/示波器可见阶梯上升与下降。

**⑤ 切换为「GD25Qxx SPI Flash」例程后**：依次以硬件 SPI / 软件 SPI 探测 Flash、擦扇区、Quad 写读 512 字节比对，打印 `HARDWARE SPI : PASS` / `SOFTWARE SPI : PASS`。

**⑥ 切换为「I2S 音频播放」例程后**：以硬件 / 软件 I2S 播放 `wavetestdata` 测试音，打印采样率/通道/位数与 `PASS`。

### 5.3 现象对照速查（默认 `main.c` = GD32F470 Full Test）

| 操作 | 预期 LED | 预期串口 |
| --- | --- | --- |
| 上电复位（GD32470I-EVAL） | 三灯亮 → 阶段 1 按键翻转 → 阶段 2 保持 | 打印 Boot 横幅 → 阶段 1/2 → `ALL TESTS COMPLETED SUCCESSFULLY!` |
| 阶段 1 按 PA0/PC13/PB14 | 对应 LED 翻转 | 打印 `[Polling] <引脚> Pressed! LEDx Toggle.` |
| 进入中断模式后按任意键 | 对应 LED 翻转 | 无打印（中断仅翻转 LED） |
| 向串口发数据（默认 PRINTF 模式） | 无变化 | 无应答（默认 main 跑 PRINTF 模式，不接收） |

---

## 6. 目录结构

```
GD32F470IKH6/                        # 仓库根目录
├── .gitignore                      # 忽略 Objects/、Listings/、*.o/*.d/*.hex/*.map 等构建产物
├── LICENSE                         # 用户代码 MIT（Copyright (c) 2026 琪瑶luo）
├── README.md
├── RELEASE_NOTES.md                # v0.1.0 发布说明（若已发布）
└── GD32F470IKH6/                   # 源码根目录（与仓库根目录同名）
    ├── Project/                    # F470IKH6_demo.uvprojx / .uvoptx，构建产物 Objects/、Listings/
    ├── User/                       # main.c/.h、systick.c/.h、gd32f4xx_it.c/.h、gd32f4xx_libopt.h、bsp_pin_def.h
    ├── BSP/                        # bsp_led、bsp_key、bsp_usart、bsp_adc、bsp_dac、bsp_i2c、bsp_exmc、bsp_spi、bsp_i2s
    ├── App/                        # app_led、app_key、app_usart、app_adc、app_dac、app_i2c_eeprom
    ├── Components/                 # 组件层（芯片无关）
    │   └── drivers/                # drv_at24c02、drv_storage、drv_gd25qxx、drv_wav、drv_i2s_player、wave_data
    ├── OSAL/                       # 预留空目录（系统抽象层，待接入）
    ├── Services/                   # 预留空目录（服务层，待接入）
    ├── Firmware/                   # CMSIS + GD32F4xx 标准外设库 + USB 库源码（USB 未加入编译）
    ├── Doc/                        # 评估板原理图 PDF、用户指南 PDF、readme.txt 实验/例程文档
    ├── tools/                      # gen_wave.py（WAV 测试数据生成）
    └── .vscode/                    # Keil Assistant / IntelliSense 配置与日志
```

`.workbuddy/` 是本地助手的工作记录目录，与固件无关。

---

## 7. 当前状态与已知限制

- 默认 `main.c` 跑 **「GD32F470 Full Test」**：阶段 1 `app_key_init_polling()` + 轮询循环（约 5s）；阶段 2 循环 `drv_storage_run()` 三种模式（SDRAM / SDRAM 休眠 / NAND，每项约 1s）；最后 `app_key_init_exti()` 进入 EXTI 中断空闲。`app_led_*`、`app_usart_task()`、`app_adc_*`、`app_dac_scan_task()`、`app_i2c_eeprom_*`、`bsp_spi_*`/`drv_gd25qxx_*`、`bsp_i2s_*`/`drv_i2s_player_*` 都不在默认 `main.c` 里，需按第 4.2 节手动改 `main.c`。
- 默认例程依赖板载 **SDRAM / NAND**（GD32470I-EVAL 自带）；在其它板卡或存储未焊接时，阶段 2 会报错但程序仍进入中断模式，属预期。
- **按键触发方式已改为默认下降沿（按下即触发）**：`bsp_key.c` 默认 `s_key_trigger = KEY_TRIG_FALLING_PULLUP`（上拉、按下为低、EXTI 下降沿）。若改用 `bsp_key_set_trigger(KEY_TRIG_RISING_FLOATING)`，则需把按键改成按下接 3.3V 的浮空接法。旧版「松手（上升沿）触发」已不再默认。
- `app_key_polling_task()` 用 `while(...==0)` 死等松手，长按会阻塞整个主循环；`app_led_flow_task()` 为阻塞延时，约 3 秒跑完一轮。
- 双 ADC 模式需要 PC3 / PC5 接真实模拟信号；`app_adc_task_dual_dma()` 打印的是 32 位同步数据寄存器原始值，需按 GD32 手册拆分两路 ADC 的 16 位数据。
- DAC 为 12 位右对齐，输出 0~VDDA（约 3.3V）；`app_dac_scan_task()` 每 100ms 步进 100，需主循环不断调用。
- `BSP_USART_COMn = 1`，只有 `BSP_USART_COM0` 可用；`usart_typedef_enum` 的其他值会静默返回。
- EXMC 的 SDRAM/NAND 引脚由 `BSP/bsp_exmc.c` 按评估板原理图固定配置，换板需改该文件；`drv_storage_run()` 不负责引脚初始化，调用前通常由 `bsp_exmc_init_mode()` 完成。
- I2C 支持硬件（I2C0）与软件（GPIO 模拟）两套实现，均带超时保护。
- 部分源码与 `Doc/readme.txt` 的中文注释为 GBK 编码，UTF-8 编辑器直接打开会乱码（不影响编译）。
- `Firmware/` 下的 CMSIS、标准外设库、USB 库版权归 GigaDevice Semiconductor Inc.，遵循其文件头中的 BSD-3-Clause 条款；本仓库自有 `User/`、`BSP/`、`App/`、`Components/` 代码按根目录 `LICENSE` 的 MIT 许可使用。

---

## 8. 如何复核本 README（一致性依据）

| 声明 | 依据文件 |
| --- | --- |
| 默认 `main.c` 行为（Full Test：按键轮询 → EXMC 存储 → EXTI 中断） | `User/main.c`、`App/app_key.c`、`Components/drivers/drv_storage.c`、`BSP/bsp_exmc.c` |
| 引脚/时钟集中定义（LED/KEY/USART/ADC/DAC/I2C/SPI/I2S/EXMC） | `User/bsp_pin_def.h` |
| 按键回调、下降沿触发、`bsp_key_set_trigger`、上升/下降沿分发 | `BSP/bsp_key.c/.h`、`App/app_key.c`、`User/gd32f4xx_it.c` |
| USART 两模式、256 字节接收缓冲、`set_baudrate`/`send_dma`/`get_rx_data`、IRQ 回显 | `BSP/bsp_usart.c/.h`、`App/app_usart.c` |
| DAC 接口（PA4、12 位、阶梯波任务） | `BSP/bsp_dac.c/.h`、`App/app_dac.c` |
| I2C 硬/软双实现、AT24C02 组件驱动 | `BSP/bsp_i2c.c/.h`、`Components/drivers/drv_at24c02.c/.h` |
| EXMC SDRAM/NAND、`drv_storage_run` 三模式 | `BSP/bsp_exmc.c/.h`、`Components/drivers/drv_storage.c/.h` |
| SPI 硬件/软件双实现、Quad、GD25Qxx Flash 驱动 | `BSP/bsp_spi.c/.h`、`Components/drivers/drv_gd25qxx.c/.h` |
| I2S 硬件/软件、WAV 解析、I2S Player、嵌入测试音 | `BSP/bsp_i2s.c/.h`、`Components/drivers/drv_wav.c/.h`、`drv_i2s_player.c/.h`、`wave_data.c/.h` |
| ADC 三模式、TIMER1 触发、DMA1_CH0、温度/电压换算 | `BSP/bsp_adc.c/.h`、`App/app_adc.c` |
| LED/KEY/USART/ADC/DAC/I2C/SPI/I2S/EXMC 的 App 任务函数与打印字符串 | `App/*.c`、`BSP/*.c` |
| EXTI / USART0 / SPI1 中断服务位置、SysTick | `User/gd32f4xx_it.c`、`User/systick.c` |
| 器件、DFP 版本、AC6/C99/-O2、文件组、USB 库未编译 | `Project/F470IKH6_demo.uvprojx` |
| 引脚分配、JP5 跳线、MAX3232/COM0、115200 参数 | `Doc/GD32470I-EVAL评估板用户指南_V2.0.pdf` |
| 01–29 例程框架与完整 main 调用片段 | `Doc/readme.txt` |
| clone 地址 | `git remote -v`（`https://github.com/you-678/GD32F470IKH6.git`） |
