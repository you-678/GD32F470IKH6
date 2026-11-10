# GD32F470IKH6 裸机学习工程

基于 **GD32470I-EVAL 评估板（主控 GD32F470IKH6，Cortex-M4）** 的 Keil MDK-ARM 单工程，把官方分散的入门 Demo 抽成 `BSP / App / User` 三层，方便在一个工程里切换、对比 GPIO 轮询与中断、串口轮询与 DMA 等写法。

> **当前状态**：工程里唯一在运行的例程是 `04_USART_Printf` —— 上电三颗 LED 同步闪 2 次，之后主循环只轮询 Tamper 按键并 `printf`。其余函数都在源码里，但没有被 `main.c` 调用。本 README 描述的是当前工作区已编译通过的版本。

> **文档基准（2026-11-10）**：本文的行为描述逐条对照当前工作区源码与工程配置核实，依据为 `User/main.c`、`App/*.c`、`BSP/*.c/.h`、`User/gd32f4xx_it.c`、`User/gd32f4xx_libopt.h`、`Project/F470IKH6_demo.uvprojx`、`Project/Objects/F470IKH6_demo.build_log.htm` 以及 `Doc/` 下两份评估板 PDF。凡「只是骨架 / 未跑通」的能力都在第 4、7 节显式标注，**不计入已实现功能**；复核方法见第 8 节。

---

## 1. 项目解决什么问题

| 痛点 | 本仓库的做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，想对比流水灯 / 按键轮询 / EXTI 中断 / 串口打印时要反复建工程、切工程 | 把 LED、按键、USART0 抽成 `BSP/` 板级驱动，把例程逻辑抽成 `App/` 任务函数；**只改 `User/main.c` 的 `while(1)` 就能切换例程**，`BSP/`、`App/` 的 6 个 `.c` 已全部在工程文件组内 |
| 裸机调试没有日志，只能靠点灯猜状态 | 实现 `printf` 重定向到 USART0（PA9/PA10，115200-8-N-1），可直接用串口助手看输出 |
| 学习过程中「代码写了、现象没记」 | `Doc/readme.txt` 记录 01–05 例程的 main 片段、实测现象和已知缺陷（阻塞延时、长按卡死、无消抖等） |

**范围边界（避免误读）**：

- 本仓库**不是** 29 个官方例程的移植合集。`Doc/readme.txt` 里 01–05 是实验记录与代码片段，06–29 只是待办标题（ADC/DAC/RTC/CAN/USB…），仓库中没有对应实现。
- 本仓库**不是**固件产品或飞控代码，不含 RTOS、协议栈、算法。`Firmware/GD32F4xx_usb_library/` 虽带官方 USB Device/Host 库源码，但 `uvprojx` 中引用数为 0，**未加入编译、无例程调用**。
- 验证环境：µVision V5.40.0.0 / MDK-ARM Plus 5.40.0.3 / **ARMCLANG V6.19（AC6，C99，-O2）** / `GigaDevice.GD32F4xx_DFP 3.2.0`；本机最近一次构建 `0 Error(s), 0 Warning(s)`，Code=3448 / RO=532 / RW=8 / ZI=1160（构建日志是本地文件，**不在版本库内**）。

---

## 2. 主要功能

| 模块 | 真实实现 | 关键文件 | 状态 |
| --- | --- | --- | --- |
| LED BSP | 3 个 GPIO 输出：LED1=PE2、LED2=PE3、LED3=PF10（置高点亮、置低熄灭，推挽 50MHz）；提供 `init/on/off/toggle` | `BSP/bsp_led.c/.h` | 可用 |
| KEY BSP | 3 个按键：Wakeup=PA0(K2)、Tamper=PC13(K3)、User=PB14(K4)，按下为低电平；外部 10K 上拉，代码用 `GPIO_PUPD_NONE` 未开内部上下拉；`KEY_MODE_GPIO` 输入模式 / `KEY_MODE_EXTI` 下降沿中断模式 | `BSP/bsp_key.c/.h` | 可用 |
| USART0 BSP | PA9(TX)/PA10(RX) AF7，8 数据位 / 1 停止位 / 无校验 / 无流控，波特率由参数传入；`fputc`（ARMCLANG）/ `__io_putchar`（GCC）重定向 `printf`；提供 `PRINTF / IRQ / DMA` 三种模式开关和 DMA 收/发初始化函数 | `BSP/bsp_usart.c/.h` | PRINTF 可用；IRQ、DMA 见第 4.3 节 |
| LED App | `app_led_all_on/off/toggle()`、`app_led_flow_task()`（1 秒/颗流水，阻塞 3 秒）、`app_led_flash(n)`（灭 500ms → 亮 500ms，循环 n 次，上电自检） | `App/app_led.c` | 可用，当前 main 只调用 `flash(2)` |
| KEY App | `app_key_scan_task()`：3 键轮询 + 100ms 消抖 + 翻转对应 LED + 死等松手 | `App/app_key.c` | 可用，当前 main 未调用 |
| USART App | `app_usart_task()`：轮询 Tamper 键，两次 50ms 消抖；按住 → 点亮 LED3 并持续 `printf`，松开 → 熄灭 LED3 | `App/app_usart.c` | 当前唯一运行的应用任务 |
| 系统 / 中断 | `systick_config()` 配 1kHz SysTick → `delay_1ms()`；异常向量兜底死循环；`EXTI0`、`EXTI10_15`、`USART0` 中断服务 | `User/systick.c`、`User/gd32f4xx_it.c` | 可用 |
| 官方固件 | CMSIS（startup + `system_gd32f4xx.c`，25MHz HXTAL 经 PLL 配 200MHz 系统时钟）+ 整套 GD32F4xx 标准外设库（27 个 `.c`），全部加入工程编译 | `Firmware/CMSIS/`、`Firmware/GD32F4xx_standard_peripheral/` | 可用 |
| 工程与资料 | Keil 工程、VS Code / Keil Assistant 配置、评估板原理图与用户指南 PDF | `Project/`、`.vscode/`、`Doc/` | — |

**状态图例**：`可用` = 当前编译进固件且有调用路径；`当前 main 未调用` = 源码在、需按第 4.2 节手动切换；`骨架` = 只写了一半（如 IRQ 接收、DMA 收发），见第 4.3 节。

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

- **板卡**：GD32470I-EVAL（GD32F470IKH6），Mini USB 或 DC-005 提供 5V（来源：用户指南 V2.0 第 1 节）。
- **下载**：工程调试适配器配置为 CMSIS-DAP「Any」/ SW-DP（`CMSIS_AGDI.dll`），未锁定具体型号；板载 GD-Link 或外置 J-Link / CMSIS-DAP 均可，需在 `Options for Target → Debug` 里选定实际调试器。
- **串口（关键）**：USART0 = PA9(TX) / PA10(RX)。评估板经 MAX3232 引出到 DB9 **COM0**，**必须用跳线帽把 JP5 短接 1-2 选择 USART0**（短接 2-3 是 USB_FS）。COM0 是 **RS232 电平**，请用 USB-RS232 串口线；如果用 USB-TTL，请直接接 PA9 / PA10 / GND 的 **TTL 电平**，不要接 DB9（来源：用户指南 V2.0 第 15 页原理图、第 30 页串口参数）。
- 引脚分配与代码完全一致（来源：用户指南 V2.0 表 2-1，第 8 页；按键上拉电阻见原理图 V1.0 第 6 页）：

| 功能 | 引脚 | 功能 | 引脚 |
| --- | --- | --- | --- |
| LED1 | PE2 | Wakeup 键 K2 | PA0 |
| LED2 | PE3 | Tamper 键 K3 | PC13 |
| LED3 | PF10 | User 键 K4 | PB14 |
| USART0_TX | PA9 | USART0_RX | PA10 |

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
| 串口无输出 | 依次查：JP5 是否短接 1-2、串口线是否接 COM0、终端是否 115200-8-N-1、是否误把 RS232 口接到 USB-TTL；最后确认按住 Tamper ≥100ms（短按被消抖滤掉） |
| 中文注释乱码 | 部分源码与 `Doc/readme.txt` 是 GBK 编码，用 GBK / GB18030 打开即可，不影响编译 |
| `Objects/` 里有 `app_usart_dma.o`、`bsp_usart_irq.o` 等旧文件 | 历史实验残留的本地构建产物，已被 `.gitignore` 忽略且**不在当前工程文件组内**，不参与链接；`Project → Clean Targets` 可清掉 |
| VS Code Keil Assistant 提示「发现无效的文件组 Group: App / Middlewares」 | 插件解析告警，`Middlewares` 确为空组；不影响 µVision 编译 |

---

## 4. 使用方法

### 4.1 跑通当前例程（04_USART_Printf）

1. JP5 短接 1-2，串口线接 COM0，打开串口助手：**115200 / 8 数据位 / 1 停止位 / 无校验 / 无流控**。
2. 编译（F7）→ 下载（F8）→ 复位。
3. 上电观察：三颗 LED 同步闪 2 次（约 2 秒），**串口此时静默**。
4. 按住 **Tamper 键（K3 / PC13）≥100ms**：LED3 常亮，串口持续刷屏；松开即停。

### 4.2 切换例程

01/02/03 **只改 `User/main.c` 的 `while(1)`**（初始化保持现状）；05 是例外，需同时改串口模式并补 `extern` 声明。改完重新 Build + Download，`App/`、`BSP/` 的 6 个 `.c` 无需增删。`Doc/readme.txt` 里有 01–05 的完整片段，下面四个可直接复制：

```c
/* 01 流水灯：LED1→LED2→LED3，1 秒/颗，阻塞 3 秒一轮 */
while(1) { app_led_flow_task(); }

/* 02 按键轮询：Wakeup/Tamper/User 分别翻转 LED1/LED2/LED3 */
while(1) { app_key_scan_task(); }

/* 03 EXTI 中断：把三个 bsp_key_init 的 KEY_MODE_GPIO 改成 KEY_MODE_EXTI；
   ISR 已在 User/gd32f4xx_it.c 中实现，主循环留空 */
app_led_flash(1);
while(1) { }

/* 05 串口中断回显：把整段替换 User/main.c
   （原文件已 include gd32f4xx_it.h / gd32f4xx.h；后者自动带出 gd32f4xx_libopt.h 与 string.h） */
#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"

extern uint8_t           rx_buffer[RX_BUFFER_SIZE];   /* 定义在 User/gd32f4xx_it.c */
extern volatile uint16_t rx_index;
extern volatile uint8_t  rx_complete_flag;

int main(void)
{
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);   /* 模式 PRINTF -> IRQ */

    while(1) {
        if(rx_complete_flag) {
            printf("IRQ Recv: %s", (const char *)rx_buffer);
            memset(rx_buffer, 0, sizeof(rx_buffer));
            rx_index = 0;
            rx_complete_flag = 0;
        }
    }
}
```

05 的 `rx_buffer[128]`、`rx_index`、`rx_complete_flag` 定义在 `User/gd32f4xx_it.c`（`RX_BUFFER_SIZE=128` 在 `gd32f4xx_it.h`），`USART0_IRQHandler` 收到 `\n` 才置帧完成标志 —— **串口助手必须以换行结尾发送，否则永远不回显**；`printf` 仍走 `fputc` 轮询发送，不依赖 DMA。

### 4.3 串口三种模式的真实状态

`bsp_usart_init()` 第 2 个参数：

| 模式 | 代码做了什么 | 能否当功能用 |
| --- | --- | --- |
| `BSP_USART_MODE_PRINTF` | 仅轮询发送，`printf` 重定向到 USART0；不使能任何接收中断 | ✅ 当前 main 使用，**只发不收** |
| `BSP_USART_MODE_IRQ` | 使能 RBNE 中断 + NVIC；`USART0_IRQHandler` 把字节存入 `rx_buffer[128]`，遇到 `\n` 置 `rx_complete_flag` | ⚠️ 当前 main 没有消费该标志，**不会回显**；按 4.2 节的 05 片段改 main 后才回显 |
| `BSP_USART_MODE_DMA` | `bsp_usart_dma_rx_init()` 配 DMA1_CH2（循环 + IDLE），`bsp_usart_dma_tx_start()` 配 DMA1_CH7 | ❌ IDLE 帧结束 ISR 与 DMA1_CH7 完成 ISR 在 `gd32f4xx_it.c` 中被注释，且当前 main 的 DMA 回显逻辑整段被注释，**DMA 回显链路未跑通** |

> **串口号不可随便改**：`usart_typedef_enum` 虽有 `BSP_USART_COM0/1/2` 三个枚举，但 `bsp_usart.c` 的查表数组只有 COM0 一组，`COM_COUNT = 1`，`bsp_usart_init()` 首行 `if (usart_num >= COM_COUNT) return;`。**传 COM1 / COM2 会静默不初始化**，只有 COM0 可用；要扩展需先补齐 6 个查表数组。

### 4.4 常用接口速查

```c
void bsp_led_init(led_typedef_enum lednum);            /* LED1/LED2/LED3 */
void bsp_led_on/off/toggle(led_typedef_enum lednum);
void bsp_key_init(key_typedef_enum k, keymode_typedef_enum mode);
uint8_t bsp_key_state_get(key_typedef_enum k);         /* 0 = 按下 */
void bsp_usart_init(usart_typedef_enum com, usartmode_typedef_enum mode, uint32_t baudrate);
void bsp_usart_dma_rx_init(void);
void bsp_usart_dma_tx_start(uint8_t *data, uint16_t len);
void app_led_all_on/off/toggle(void);
void app_led_flow_task(void);  void app_led_flash(uint8_t times);
void app_key_scan_task(void);  void app_usart_task(void);
void systick_config(void);     void delay_1ms(uint32_t ms);
```

这些声明由 `User/gd32f4xx_libopt.h` 统一汇总（含标准外设库头、`stdio.h`/`string.h`、`systick.h` 和全部 `BSP/`、`App/` 头），`gd32f4xx.h` 会自动包含它，因此各 `.c` 只要 `#include "gd32f4xx.h"` 就能拿到全部声明。

---

## 5. 输入输出示例

### 5.1 输入

| 输入 | 形式 | 代码路径 / 说明 |
| --- | --- | --- |
| 上电 / 复位 | 硬件 | `systick_config()` → 3 个 LED、3 个按键初始化 → `bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U)` → `app_led_flash(2)` → 进入 `while(1)` |
| **Tamper 键（K3 / PC13）按下** | 低电平，板上 10K 上拉 | `app_usart_task()` 连续两次 50ms 消抖，第三次仍为低才确认，**需按住 ≥100ms** |
| Tamper 键松开 | 高电平 | 走 `else` 分支 `bsp_led_off(LED3)`，停止打印 |
| 串口助手下发字符 | 无 | 当前 PRINTF 模式未使能接收中断，**不接收、不回显**；Wakeup / User 键、流水灯同样未接入主循环 |
| 代码层参数 | `User/main.c` | 串口号 COM0（唯一可用）、模式 PRINTF、波特率 115200；改模式/波特率有效，改串口号无效（见 4.3） |
| 切换到 05 后 | 串口助手发送 | 必须带 `\n` 结尾，例如 `hello\n`，才置位 `rx_complete_flag` 并回显 `IRQ Recv: hello` |

### 5.2 输出

**① 上电自检时间轴**（`app_led_flash(2)`：灭 500ms → 亮 500ms，循环 2 次）

| 时刻 | LED1/2/3 | 串口 |
| --- | --- | --- |
| 0 ~ 500ms | 全灭 | 静默 |
| 500ms ~ 1s | 全亮 | 静默 |
| 1 ~ 1.5s | 全灭 | 静默 |
| 1.5 ~ 2s | 全亮 | 静默 |
| ≥ 2s | 全灭，进入主循环 | 静默 |

> 注意：官方 04 demo 会在上电时先打印一行 `USART printf example: please press the Tamper key`（用户指南 V2.0 第 24 页），**本工程 `main.c` 删掉了这句**，所以上电串口没有任何输出 —— 以本仓库代码为准。

**② 按住 Tamper 键**：LED3 常亮，每轮 `app_usart_task()` 经过 100ms 消抖后调用一次 `printf`，约每 100ms 输出一行：

```
 USART printf example 
 USART printf example 
 USART printf example 
 ...（按住期间持续刷屏）
```

源码（`App/app_usart.c`）与实际字节流：

```c
bsp_led_on(LED3);
printf("\r\n USART printf example \r\n");
```

```
可见文本: "\r\n USART printf example \r\n"          共 26 字节
十六进制: 0D 0A 20 55 53 41 52 54 20 70 72 69 6E 74
          66 20 65 78 61 6D 70 6C 65 20 0D 0A
```

**③ 松开 Tamper 键**：LED3 熄灭，串口停止输出，主循环继续空转轮询。

**④ 构建输出**（`Project/Objects/F470IKH6_demo.build_log.htm`，本机 2026-11-09 记录）：

```
Program Size: Code=3448 RO-data=532 RW-data=8 ZI-data=1160
FromELF: creating hex file...
".\Objects\F470IKH6_demo.axf" - 0 Error(s), 0 Warning(s).
```

**⑤ 切换例程后的预期输出**（改动位置：仅 `User/main.c`）：

| 例程 | 输入 | 输出 |
| --- | --- | --- |
| 01 流水灯 | 上电 | LED1→LED2→LED3 各亮 1 秒循环；串口静默 |
| 02 按键轮询 | 按 K2 / K3 / K4 | 分别翻转 LED1 / LED2 / LED3；串口静默 |
| 03 EXTI 中断 | 按 K2 / K3 / K4 | 中断内分别翻转 LED1 / LED2 / LED3；串口静默 |
| 05 IRQ 回显 | 串口发 `hello\n` | 回显 `IRQ Recv: hello`；不发 `\n` 无应答 |

### 5.3 现象对照速查（当前 main）

| 操作 | 预期 LED | 预期串口 |
| --- | --- | --- |
| 上电 | 三灯同步闪 2 次后全灭 | 静默 |
| 点按 Tamper（<100ms） | 无变化 | 无输出（没通过消抖窗口） |
| 按住 Tamper（≥100ms） | LED3 常亮 | 每约 100ms 重复打印 `USART printf example`（26 字节/行） |
| 松开 Tamper | LED3 熄灭 | 停止输出 |
| 按 Wakeup / User | 无变化 | 无输出 |
| 向串口发数据 | 无变化 | 无应答 |

---

## 6. 目录结构

```
GD32F470IKH6/                        # 仓库根目录
├── .gitignore                      # 忽略 Objects/、Listings/、*.o/*.d/*.hex/*.map 等构建产物
├── LICENSE                         # 用户代码 MIT（Copyright (c) 2026 琪瑶luo）
├── README.md
└── GD32F470IKH6/                   # 源码根目录（与仓库根目录同名）
    ├── Project/                    # F470IKH6_demo.uvprojx / .uvoptx，构建产物 Objects/、Listings/
    ├── User/                       # main.c/.h、systick.c/.h、gd32f4xx_it.c/.h、gd32f4xx_libopt.h
    ├── BSP/                        # bsp_led、bsp_key、bsp_usart（板级驱动）
    ├── App/                        # app_led、app_key、app_usart（例程任务函数）
    ├── Firmware/                   # CMSIS + GD32F4xx 标准外设库 + USB 库源码（USB 未加入编译）
    ├── Middlewares/                # 空目录（工程里是空文件组，历史中间件已移除）
    ├── Doc/                        # 评估板原理图 PDF、用户指南 PDF、readme.txt 实验笔记
    └── .vscode/                    # Keil Assistant / IntelliSense 配置与日志
```

`.workbuddy/` 是本地助手的工作记录目录，与固件无关。

---

## 7. 当前状态与已知限制

- 主循环只调用 `app_usart_task()`；`app_led_flow_task()`、`app_key_scan_task()`、Wakeup / User 键都不跑，需按第 4.2 节手动切换。
- `app_usart.h` 声明了 `void app_usart_init(void);`，但**全仓库没有实现**，调用它会产生链接错误；串口初始化请用 `bsp_usart_init()`。
- `app_key_scan_task()` 用 `while(RESET == bsp_key_state_get(...))` 死等松手，长按会阻塞整个主循环；`app_led_flow_task()` 用 3 秒阻塞延时且首段漏关 LED2，可能出现「双灯同亮 1 秒」。这些缺陷在 `Doc/readme.txt` 里有原始记录。
- EXTI 模式只有硬件中断，没有消抖 / 松手处理，机械抖动可能导致 LED 反复翻转。
- IRQ / DMA 串口模式的状态见第 4.3 节，**不要当作已完成的回显功能**。`BSP_USART_MODE_DMA` 分支里 `usart_dma_receive_config(periph, USART_DENT_ENABLE)` 误用了发送 DMA 宏（应为 `USART_DENR_ENABLE`），虽然后续 `bsp_usart_dma_rx_init()` 会用正确宏重配，但该分支单独使用时接收 DMA 请求不会被置位。
- `BSP_USART_COM1 / COM2` 是占位枚举，无对应查表数据，传入 `bsp_usart_init()` 会静默返回。
- Git 历史提交里还留有 `app_usart_irq.c/.h`、`app_usart_dma.c/.h`、`bsp_usart_irq.*`、`bsp_usart_dma.*` 和 `Middlewares/middleware_usart*`（内容基本被注释），当前工作区已从工程文件组移除/删除；它们是实验残留，不参与 `main.c` 的行为。`Objects/` 下同名 `.o` 也是残留构建产物。
- 部分源码与 `Doc/readme.txt` 的中文注释为 GBK 编码，UTF-8 编辑器直接打开会乱码（不影响编译）。
- `Firmware/` 下的 CMSIS、标准外设库、USB 库版权归 GigaDevice Semiconductor Inc.，遵循其文件头中的 BSD-3-Clause 条款；本仓库自有 `User/`、`BSP/`、`App/` 代码按根目录 `LICENSE` 的 MIT 许可使用。

---

## 8. 如何复核本 README（一致性依据）

| 声明 | 依据文件 |
| --- | --- |
| 当前 main 行为、上电自检次数、串口模式与波特率 | `User/main.c` |
| 按键消抖 100ms、打印字符串与 LED3 动作 | `App/app_usart.c` |
| LED/KEY/USART 引脚、三种模式、COM_COUNT、DMA 通道、printf 重定向 | `BSP/bsp_led.h`、`BSP/bsp_key.h`、`BSP/bsp_usart.c/.h` |
| EXTI / USART0 / SysTick ISR，DMA 与 FTF ISR 被注释，`rx_buffer[128]` | `User/gd32f4xx_it.c/.h`、`User/systick.c` |
| 器件、DFP 版本、AC6/C99/-O2、文件组、USB 库未编译 | `Project/F470IKH6_demo.uvprojx` |
| 编译器版本、0 Error 0 Warning、Code/RO/RW/ZI | `Project/Objects/F470IKH6_demo.build_log.htm`（本机文件，未入库） |
| 引脚分配、K2/K3/K4 编号、JP5 跳线、MAX3232/COM0、115200 参数、官方 04 例程现象 | `Doc/GD32470I-EVAL评估板用户指南_V2.0.pdf`（表 2-1、第 15/24/30 页）、`Doc/GD32F470I-EVAL-V1.0.pdf`（第 6 页按键上拉） |
| 01–05 实验记录、06–29 待办标题 | `Doc/readme.txt`（GBK） |
| clone 地址 | `git remote -v`（`https://github.com/you-678/GD32F470IKH6.git`） |
