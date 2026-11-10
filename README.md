# GD32F470IKH6 裸机学习工程

基于 **GD32470I-EVAL 评估板（主控 GD32F470IKH6，Cortex-M4）** 的 Keil MDK-ARM 单工程，把官方分散的入门 Demo 抽成 `BSP / App / User` 三层，方便在一个工程里切换、对比 GPIO 轮询与中断、串口轮询与 DMA 等写法。

> **当前状态**：工程里唯一在跑的例程是 `04_USART_Printf` —— 上电 LED 闪 2 次，之后主循环只轮询 TAMPER 按键并 `printf`。其余函数都在源码里，但没有被 `main.c` 调用。本 README 描述的是当前工作区已编译通过的版本。

---

## 1. 项目解决什么问题

| 痛点 | 本仓库的做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，想对比流水灯 / 按键轮询 / EXTI 中断 / 串口打印时要反复建工程、切工程 | 把 LED、按键、USART0 抽成 `BSP/` 板级驱动，把例程逻辑抽成 `App/` 任务函数；**只改 `User/main.c` 的 `while(1)` 就能切换例程** |
| 裸机调试没有日志，只能靠点灯猜状态 | 实现 `printf` 重定向到 USART0（PA9/PA10，115200-8-N-1），可直接用串口助手看输出 |
| 学习过程中「代码写了、现象没记」 | `Doc/readme.txt` 记录 01–05 例程的 main 片段、实测现象和已知缺陷（阻塞延时、长按卡死、无消抖等） |

**范围边界（避免误读）**：

- 本仓库**不是** 29 个官方例程的移植合集。`Doc/readme.txt` 里 01–05 是实验记录，06–29 只是待办标题（ADC/DAC/RTC/CAN/USB…），仓库中没有对应实现。
- 本仓库**不是**固件产品或飞控代码，不含 RTOS、协议栈、算法。`Firmware/GD32F4xx_usb_library/` 虽带官方 USB Device/Host 库源码，但**未加入工程编译、无例程调用**。
- 验证环境：µVision V5.40.0.0 / MDK-ARM 5.40 / **ARMCLANG V6.19（AC6，C99）** / `GigaDevice.GD32F4xx_DFP 3.2.0`；最近一次构建 `0 Error(s), 0 Warning(s)`，Code=3448 / RO=532 / RW=8 / ZI=1160。

---

## 2. 主要功能

| 模块 | 真实实现 | 关键文件 | 状态 |
| --- | --- | --- | --- |
| LED BSP | 3 个 GPIO 输出：LED1=PE2、LED2=PE3、LED3=PF10（输出高点亮、低熄灭），提供 `init/on/off/toggle` | `BSP/bsp_led.c/.h` | 可用 |
| KEY BSP | 3 个按键：Wakeup=PA0、Tamper=PC13、User=PB14（低电平有效，板上 10K 上拉）；`KEY_MODE_GPIO` 输入模式 / `KEY_MODE_EXTI` 下降沿中断模式 | `BSP/bsp_key.c/.h` | 可用 |
| USART0 BSP | PA9(TX)/PA10(RX) AF7，8 数据位 / 1 停止位 / 无校验 / 无流控，波特率由参数传入；`fputc`（ARMCLANG）/ `__io_putchar`（GCC）重定向 `printf`；提供 `PRINTF / IRQ / DMA` 三种模式开关和 DMA 收/发初始化函数 | `BSP/bsp_usart.c/.h` | PRINTF 可用；IRQ、DMA 见第 4 节限制 |
| LED App | `app_led_all_on/off/toggle()`、`app_led_flow_task()`（1 秒/颗流水）、`app_led_flash(n)`（亮 500ms / 灭 500ms × n，上电自检） | `App/app_led.c` | 可用，当前 main 只调用 `flash` |
| KEY App | `app_key_scan_task()`：3 键轮询 + 100ms 消抖 + 翻转对应 LED + 死等松手 | `App/app_key.c` | 可用，当前 main 未调用 |
| USART App | `app_usart_task()`：轮询 TAMPER 键，两次 50ms 消抖；按住 → 点亮 LED3 并持续 `printf`，松开 → 熄灭 LED3 | `App/app_usart.c` | 当前唯一运行的应用任务 |
| 系统 / 中断 | `systick_config()` 配 1kHz SysTick → `delay_1ms()`；异常向量兜底死循环；`EXTI0`、`EXTI10_15`、`USART0` 中断服务 | `User/systick.c`、`User/gd32f4xx_it.c` | 可用 |
| 官方固件 | CMSIS（含 startup + `system_gd32f4xx.c`，系统时钟 200MHz / 25MHz HXTAL PLL）+ 整套 GD32F4xx 标准外设库，全部加入工程编译 | `Firmware/CMSIS/`、`Firmware/GD32F4xx_standard_peripheral/` | 可用 |
| 工程与资料 | Keil 工程、VS Code / Keil Assistant 配置、评估板原理图与用户指南 PDF | `Project/`、`.vscode/`、`Doc/` | — |

---

## 3. 安装方法

### 3.1 软件环境

| 项 | 要求 |
| --- | --- |
| 操作系统 | Windows（Keil µVision 工程，无 Makefile / CMake 构建链） |
| IDE / 编译器 | Keil MDK-ARM 5.40，ARMCLANG V6.19（AC6，工程已勾选 `uAC6`、`C99`）——其他 AC6 版本未验证 |
| 器件支持包 | `GigaDevice.GD32F4xx_DFP 3.2.0`（工程选定器件 `GD32F470IK`，Flash 算法 `GD32F4xx_3MB.FLM`） |
| 串口终端 | 任意支持 115200 / 8-N-1 的串口助手 |
| 可选 | VS Code + Keil Assistant 插件（`.vscode/` 已带 IntelliSense 配置） |

### 3.2 硬件与接线

- **板卡**：GD32470I-EVAL（GD32F470IKH6），Mini USB 或 DC-005 提供 5V。
- **下载**：板载 GD-Link（SWD）或 J-Link；工程当前为硬件调试模式，Flash 算法为 3MB @ `0x08000000`。
- **串口（关键）**：USART0 = PA9(TX) / PA10(RX)。评估板经 MAX3232 引出到 DB9 **COM0**，**必须用跳线帽把 JP5 短接 1-2 选择 USART0**（短接 2-3 是 USB_FS_VBUS）。COM0 是 **RS232 电平**，请用 USB-RS232 串口线；如果用 USB-TTL，请直接接 PA9 / PA10 / GND 的 **TTL 电平**，不要接 DB9。
- 引脚分配与代码完全一致（来源：`Doc/GD32470I-EVAL评估板用户指南_V2.0.pdf` 表 2-1、图 4-3/4-4/4-5）：

| 功能 | 引脚 | 功能 | 引脚 |
| --- | --- | --- | --- |
| LED1 | PE2 | Wakeup 键 | PA0 |
| LED2 | PE3 | Tamper 键 | PC13 |
| LED3 | PF10 | User 键 | PB14 |
| USART0_TX | PA9 | USART0_RX | PA10 |

### 3.3 获取与编译

```bash
git clone https://github.com/you-678/GD32F470IKH6.git
cd GD32F470IKH6
```

1. 用 µVision 打开 `GD32F470IKH6/Project/F470IKH6_demo.uvprojx`（Target：`GD32F470IKH6_demo`）。
2. `Project → Build Target`（F7）。产物在 `GD32F470IKH6/Project/Objects/`：`F470IKH6_demo.axf`、`F470IKH6_demo.hex`、`.map`。`.gitignore` 忽略了 `Objects/`、`Listings/`、`*.o`、`*.hex`，**clone 后必须自行编译才有固件**。
3. `Download`（或把 `.hex` 烧进 Flash）后复位运行。

**常见报错**

| 现象 | 处理 |
| --- | --- |
| `#error "Please select the target GD32F4xx device in gd32f4xx.h file"` | `gd32f4xx.h` 里的器件宏是注释状态，正常由 DFP 自动注入；若换环境后报此错，在 `Options for Target → C/C++ (AC6) → Define` 添加 `GD32F470` |
| 串口无输出 | 先查 JP5 是否短接 1-2、串口线是否接 COM0、终端是否 115200-8-N-1，再查是否误把 RS232 口接到 USB-TTL |
| 中文注释乱码 | 部分源码 / `Doc/readme.txt` 是 GBK 编码，用 GBK 或 GB18030 打开即可 |

---

## 4. 使用方法

**跑当前例程（04_USART_Printf）**：接线 → 编译下载 → 打开串口助手（115200, 8N1, 无流控）→ 上电 → 按住 **Tamper 键（K3 / PC13）** 观察输出 → 松开。

**切换例程**：只改 `User/main.c` 的 `while(1)`（初始化保持现状即可，`Doc/readme.txt` 里有 01–05 的完整片段）：

```c
/* 01 流水灯：LED1→LED2→LED3，1 秒/颗，阻塞延时 3 秒 */
while(1) { app_led_flow_task(); }

/* 02 按键轮询：三键分别翻转 LED1/LED2/LED3 */
while(1) { app_key_scan_task(); }

/* 03 EXTI 中断：把三个 bsp_key_init 的 KEY_MODE_GPIO 改成 KEY_MODE_EXTI，
   ISR 已在 User/gd32f4xx_it.c 中实现，主循环留空 */
app_led_flash(1);
while(1) { }
```

修改后重新 Build + Download 即可；`App/`、`BSP/` 的 6 个 `.c` 默认都在工程文件组内，无需增删文件。

**串口三种模式的真实状态**（`bsp_usart_init()` 第 2 个参数）：

| 模式 | 代码做了什么 | 能否当功能用 |
| --- | --- | --- |
| `BSP_USART_MODE_PRINTF` | 仅轮询发送，`printf` 重定向到 USART0 | ✅ 当前 main 使用 |
| `BSP_USART_MODE_IRQ` | 使能 RBNE 中断 + NVIC；`USART0_IRQHandler` 把字节存入 `rx_buffer[128]`，遇到 `\n` 置 `rx_complete_flag` | ⚠️ 主循环没有消费该标志，**不会回显**，只是中断接收骨架 |
| `BSP_USART_MODE_DMA` | 使能 USART DMA 收发；`bsp_usart_dma_rx_init()` 配 DMA1_CH2（循环 + IDLE），`bsp_usart_dma_tx_start()` 配 DMA1_CH7 | ❌ IDLE 帧结束 ISR 与 DMA1_CH7 完成 ISR 在 `gd32f4xx_it.c` 中被注释，DMA 回显链路**未跑通** |

**常用接口速查**

```c
void bsp_led_init(led_typedef_enum lednum);            /* LED1/LED2/LED3 */
void bsp_led_on/off/toggle(led_typedef_enum lednum);
void bsp_key_init(key_typedef_enum k, keymode_typedef_enum mode);
uint8_t bsp_key_state_get(key_typedef_enum k);         /* 0 = 按下 */
void bsp_usart_init(usart_typedef_enum com, usartmode_typedef_enum mode, uint32_t baudrate);
void systick_config(void);  void delay_1ms(uint32_t ms);
```

---

## 5. 输入输出示例

### 5.1 输入

| 输入 | 形式 | 说明 |
| --- | --- | --- |
| 上电 / 复位 | 硬件 | 触发 `systick_config()` → 3 个 LED / 3 个按键初始化 → `bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U)` → `app_led_flash(2)` |
| **Tamper 键（PC13）按下** | 低电平，板上 10K 上拉 | `app_usart_task()` 连续两次 50ms 消抖，**需按住 ≥100ms** 才确认，随后点亮 LED3 并打印 |
| Tamper 键松开 | 高电平 | LED3 熄灭，停止打印 |
| 串口助手下发字符 | 无 | 当前 PRINTF 模式未使能接收中断，**不发也不回显**；Wakeup / User 键、流水灯同样未接入主循环 |
| 代码层参数 | `main.c` | 串口号 COM0、模式 PRINTF、波特率 115200；改这三个参数即可换配置 |

### 5.2 输出

**① 上电**：LED1/LED2/LED3 同步闪 **2 次**（亮 500ms → 灭 500ms，约 2 秒），随后全灭；**串口此时没有任何输出**（注意：官方 04 demo 会先打印一行 `USART printf example: please press the Tamper key`，本工程删掉了这句，以本仓库代码为准）。

**② 按住 Tamper 键**：LED3 常亮，串口每约 100ms 输出一行（实际字节为 `\r\n USART printf example \r\n`）：

```
 USART printf example 
 USART printf example 
 USART printf example 
 ...（按住期间持续刷屏）
```

对应的源码行为（`App/app_usart.c`）：

```c
bsp_led_on(LED3);
printf("\r\n USART printf example \r\n");
```

**③ 松开 Tamper 键**：LED3 熄灭，串口停止输出，主循环继续空转轮询。

**④ 构建输出**（`Project/Objects/F470IKH6_demo.build_log.htm`）：

```
Program Size: Code=3448 RO-data=532 RW-data=8 ZI-data=1160
FromELF: creating hex file...
".\Objects\F470IKH6_demo.axf" - 0 Error(s), 0 Warning(s).
```

### 5.3 现象对照速查

| 操作 | 预期 LED | 预期串口 |
| --- | --- | --- |
| 上电 | 三灯同步闪 2 次后全灭 | 静默 |
| 点按 Tamper（<100ms） | 无变化 | 无输出（没通过消抖窗口） |
| 按住 Tamper（≥100ms） | LED3 常亮 | 每约 100ms 重复打印 `USART printf example` |
| 按 Wakeup / User | 无变化 | 无输出 |
| 向串口发数据 | 无变化 | 无应答 |

---

## 6. 目录结构

```
GD32F470IKH6/
├── .gitignore                # 忽略 Objects/、Listings/、*.o/*.d/*.hex 等构建产物
├── LICENSE                   # 用户代码 MIT（Copyright (c) 2026 琪瑶luo）
├── README.md
└── GD32F470IKH6/
    ├── Project/              # F470IKH6_demo.uvprojx / .uvoptx，构建产物 Objects/、Listings/
    ├── User/                 # main.c、main.h、systick.c/.h、gd32f4xx_it.c/.h、gd32f4xx_libopt.h
    ├── BSP/                  # bsp_led、bsp_key、bsp_usart（板级驱动）
    ├── App/                  # app_led、app_key、app_usart（例程任务函数）
    ├── Firmware/             # CMSIS + GD32F4xx 标准外设库 + USB 库源码（USB 未加入编译）
    ├── Middlewares/          # 空目录（历史中间件文件已移除）
    ├── Doc/                  # 评估板原理图 PDF、用户指南 PDF、readme.txt 实验笔记
    └── .vscode/              # Keil Assistant / IntelliSense 配置
```

`User/gd32f4xx_libopt.h` 是统一头文件：它包含标准外设库头、`stdio.h` 和全部 `BSP/`、`App/` 头文件，因此各 `.c` 只要 `#include "gd32f4xx.h"` 就能拿到其余声明。

---

## 7. 当前状态与已知限制

- 主循环只调用 `app_usart_task()`；`app_led_flow_task()`、`app_key_scan_task()`、Wakeup / User 键都不跑，需要按第 4 节手动切换。
- `app_key_scan_task()` 用 `while(RESET == bsp_key_state_get(...))` 死等松手，长按会阻塞整个主循环；`app_led_flow_task()` 用 3 秒阻塞延时且首段漏关 LED2，可能出现「双灯同亮 1 秒」。这些缺陷在 `Doc/readme.txt` 里有原始记录。
- EXTI 模式只有硬件中断，没有消抖 / 松手处理，机械抖动可能导致 LED 反复翻转。
- IRQ / DMA 串口模式的状态见第 4 节表格，**不要当作已完成的回显功能**。
- Git 历史提交里还留有 `app_usart_irq.c/.h`、`app_usart_dma.c/.h`、`bsp_usart_irq.*`、`bsp_usart_dma.*` 和 `Middlewares/middleware_usart*`（内容基本被注释），当前工作区已从工程文件组移除/删除；它们是实验残留，不参与 `main.c` 的行为。
- 部分源码与 `Doc/readme.txt` 的中文注释为 GBK 编码，UTF-8 编辑器直接打开会乱码（不影响编译）。
- `Firmware/` 下的 CMSIS、标准外设库、USB 库版权归 GigaDevice Semiconductor Inc.，遵循其文件头中的 BSD-3-Clause 条款；本仓库自有 `User/`、`BSP/`、`App/` 代码按根目录 `LICENSE` 的 MIT 许可使用。
