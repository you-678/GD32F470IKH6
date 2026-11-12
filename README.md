# GD32F470IKH6 裸机学习工程

基于 **GD32470I-EVAL 评估板（主控 GD32F470IKH6，Cortex-M4）** 的 Keil MDK-ARM 单工程，把官方分散的入门 Demo 抽成 `BSP / App / User` 分层，并引入 `Components/drivers` 组件层与 `bsp_pin_def.h` 集中引脚定义，方便在一个工程里切换、对比 GPIO 轮询与中断、串口轮询与中断/DMA、单/双 ADC 同步、DAC 输出、I2C 读写 EEPROM 等写法。**换板子只改 `bsp_pin_def.h` 一个文件即可**。

> **当前状态（以 `main.c` 为准）**：工程默认运行的是 **I2C-EEPROM 自检**，且现在是**两阶段**——上电后初始化 USART0（打印进度），先以**硬件 I2C0**（`BSP_I2C_MODE_HW`）初始化并探测板载/外接的 AT24C02，做「整片写入 0x00~0xFF → 整片读回比对」一致性测试，延时 1s 后再以**软件 I2C**（`BSP_I2C_MODE_SW`，GPIO 模拟 PB6/PB7）跑同样的测试，最后打印 `Both HW & SW Tested. Entering Loop` 进入死循环。循环里的 LED 表现由**最后一次（SW）测试结果**决定：通过则三灯循环单闪（LED1→LED2→LED3 各亮 500ms），失败则三灯常亮（HW/SW 复用同一总线，接好 EEPROM 两者都过、没接两者都败）。LED / KEY / USART / ADC / DAC 的例程函数都在源码里，需按第 4.2 节手动改 `main.c` 才能跑。本 README 描述的是当前工作区版本。

> **文档基准（2026-11-12）**：本文的行为描述逐条对照当前工作区源码与工程配置核实，依据为 `User/main.c`（当前为未提交版本，运行两阶段 HW→SW I2C 自检）、`User/bsp_pin_def.h`、`App/*.c`、`BSP/*.c/.h`、`Components/drivers/drv_at24c02.c`、中断文件 `User/gd32f4xx_it.c`、`User/gd32f4xx_libopt.h`、`Project/F470IKH6_demo.uvprojx`、`Project/Objects/F470IKH6_demo.build_log.htm`（本机 2026-11-12 14:38 的构建）。凡「只是函数、未被 main 调用」的能力都在第 4、7 节显式标注，**不计入默认功能**；复核方法见第 8 节。

---

## 1. 项目解决什么问题

| 痛点 | 本仓库的做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，想对比流水灯 / 按键轮询 / EXTI 中断 / 串口打印 / 单 ADC / 双 ADC / DAC / I2C 时要反复建工程、切工程 | 把 LED、按键、USART0、ADC、DAC、I2C 抽成 `BSP/` 板级驱动，把例程逻辑抽成 `App/` 任务函数；**只改 `User/main.c` 就能切换例程**，`BSP/`、`App/`、`Components/` 的 `.c` 已全部在工程文件组内 |
| 裸机调试没有日志，只能靠点灯猜状态 | 实现 `printf` 重定向到 USART0（PA9/PA10，115200-8-N-1），可直接用串口助手看输出（含浮点，见第 3.4 节） |
| 换板子要改一堆散落的引脚宏 | 所有引脚/时钟宏集中在 `User/bsp_pin_def.h`，驱动层与业务层都从它取，换 MCU/板卡只改一处 |
| 学习过程中「代码写了、现象没记」 | `Doc/readme.txt` 记录早期 01–29 实验框架与完整 main 调用示例，作为历史参考 |

**范围边界（避免误读）**：

- 本仓库**不是** 29 个官方例程的移植合集。`Doc/readme.txt` 里 01–05 是早期实验片段，06–29 多为待办/框架标题，仓库中很多尚无对应实现。
- 本仓库**不是**固件产品或飞控代码，不含 RTOS、协议栈、算法。`Firmware/GD32F4xx_usb_library/` 虽带官方 USB Device/Host 库源码，但 `uvprojx` 中引用数为 0，**未加入编译、无例程调用**。目录里的 `OSAL/`、`Services/`、`Components/protocols`、`Components/third_party` 是**预留的空分层目录**，当前只有 `Components/drivers/drv_at24c02.c` 一个组件落地。
- 验证环境：µVision V5.40.0.0 / MDK-ARM Plus 5.40.0.3 / **ARMCLANG V6.19（AC6，C99，-O2）** / `GigaDevice.GD32F4xx_DFP 3.2.0`；本机最近一次构建（2026-11-12）`0 Error(s), 0 Warning(s)`，Code=9572 / RO=620 / RW=8 / ZI=1312（构建日志是本地文件，**不在版本库内**）。

---

## 2. 主要功能

| 模块 | 真实实现 | 关键文件 | 状态 |
| --- | --- | --- | --- |
| LED BSP+App | 3 路 GPIO 输出：LED1=PE2、LED2=PE3、LED3=PF10（置高点亮、置低熄灭，推挽 50MHz）；`init/on/off/toggle`，`app_led_all_on/off/toggle()`、`app_led_flash(n)`（亮 500ms→灭 500ms，循环 n 次）、`app_led_flow_task()`（LED1→LED2→LED3 各亮 1 秒，阻塞约 3 秒） | `BSP/bsp_led.c/.h`、`App/app_led.c` | 可用（默认 main 用其作**状态指示**：初始化时三灯熄灭，自检通过循环单闪） |
| KEY BSP+App | 3 按键：Wakeup=PA0、Tamper=PC13、User=PB14，按下为低电平；`KEY_MODE_GPIO` 输入轮询 / `KEY_MODE_EXTI` 上升沿中断；支持**注册回调函数** `bsp_key_register_callback()`，中断统一由 `bsp_key_irq_handler()` 分发 | `BSP/bsp_key.c/.h`、`App/app_key.c` | 可用（默认 main 未调用） |
| USART0 BSP+App | PA9(TX)/PA10(RX) AF7，8/1/无校验/无流控，波特率参数传入；`fputc` 重定向 `printf`；`PRINTF` / `IRQ` 两模式；`bsp_usart_set_baudrate()`（运行中改波特率）、`bsp_usart_send_dma()`（DMA 发送）、`bsp_usart_get_rx_data()`（取接收缓冲）；256 字节接收缓冲在 `bsp_usart.c` 内 | `BSP/bsp_usart.c/.h`、`App/app_usart.c` | PRINTF 可用（**默认 main 用其打印进度**）；IRQ 回显可用（需切到 IRQ 模式调用 `app_usart_task()`） |
| ADC BSP+App | `bsp_adc_init(mode)` 三模式：**内部通道**（CH16 温度/CH17 Vref/CH18 Vbat，软件触发）、**双 ADC 跟随 DMA**（CH13=PC3/CH15=PC5，TIMER1 硬件触发，DMA1_CH0 搬运）、**双 ADC 并行 DMA**（同上通道，并行同步）；提供 `trigger()` 与读内部 `idata0/1/2`、DMA `data0/1` 的接口 | `BSP/bsp_adc.c/.h`、`App/app_adc.c` | 可用（默认 main 未调用） |
| DAC BSP+App | PA4，12 位右对齐；`bsp_dac_init()`（关触发、关波形、使能输出缓冲、初始 0V）、`bsp_dac_set_value()`（0~4095 → 0~VDDA）；`app_dac_scan_task()` 每 100ms 阶梯 +100（约 +0.08V），到顶转降、到底转升，非阻塞需循环调用 | `BSP/bsp_dac.c/.h`、`App/app_dac.c` | 可用（默认 main 未调用） |
| I2C BSP | I2C0（SCL=PB6 / SDA=PB7，AF4，100kHz，开漏上拉）；**硬件 I2C**（`bsp_i2c_init(BSP_I2C_MODE_HW)`）+ **软件 I2C**（GPIO 模拟时序，两套都带超时与 `printf` 报错）对外统一接口 `bsp_i2c_write_bytes()` / `bsp_i2c_read_bytes()` / `bsp_i2c_wait_standby()` | `BSP/bsp_i2c.c/.h` | 可用（**默认 main 使用硬件模式**） |
| AT24C02 组件驱动 | 芯片无关驱动：探测在线 `at24c02_init()`、单字节读写、按页写 `at24c02_write_buffer()`（内部自动分页）、连续读 `at24c02_read_buffer()`、整片写读一致性 `at24c02_test()`（256 字节全测） | `Components/drivers/drv_at24c02.c/.h` | 可用（被 I2C App 调用） |
| I2C-EEPROM App | `app_i2c_eeprom_init(mode)`：初始化 LED（灭）、初始化 I2C、探测 AT24C02、跑一致性测试，通过置 `s_ok`；`app_i2c_eeprom_task()`：通过则三灯循环单闪，失败则三灯常亮 | `App/app_i2c_eeprom.c` | 可用（**默认 main 调用两次：先 `BSP_I2C_MODE_HW`、延时 1s 后 `BSP_I2C_MODE_SW`，循环 LED 取最后一次 SW 结果**） |
| 系统 / 中断 | `systick_config()` 配 1kHz SysTick → `delay_1ms()`；`EXTI0`/`EXTI10_15` 中断服务调用 `bsp_key_irq_handler()` 分发按键；`USART0_IRQHandler` 已下沉到 `BSP/bsp_usart.c` | `User/systick.c`、`User/gd32f4xx_it.c`、`BSP/bsp_usart.c` | 可用 |
| 官方固件 | CMSIS（startup + `system_gd32f4xx.c`，25MHz HXTAL 经 PLL 配 200MHz 系统时钟）+ 整套 GD32F4xx 标准外设库（27 个 `.c`），全部加入工程编译 | `Firmware/CMSIS/`、`Firmware/GD32F4xx_standard_peripheral/` | 可用 |
| 引脚集中定义 | 所有 LED/KEY/USART/ADC/DAC/I2C 的 GPIO 端口、引脚、时钟、AF、外设、中断号宏统一在此 | `User/bsp_pin_def.h` | 可用 |
| 工程与资料 | Keil 工程、VS Code / Keil Assistant 配置、评估板原理图与用户指南 PDF、29 例程文档 | `Project/`、`.vscode/`、`Doc/` | — |

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

- **板卡**：GD32470I-EVAL（GD32F470IKH6），Mini USB 或 DC-005 提供 5V（来源：用户指南 V2.0 第 1 节）。
- **下载**：工程调试适配器配置为 CMSIS-DAP「Any」/ SW-DP（`CMSIS_AGDI.dll`），未锁定具体型号；板载 GD-Link 或外置 J-Link / CMSIS-DAP 均可，需在 `Options for Target → Debug` 里选定实际调试器。
- **串口（关键）**：USART0 = PA9(TX) / PA10(RX)。评估板经 MAX3232 引出到 DB9 **COM0**，**必须用跳线帽把 JP5 短接 1-2 选择 USART0**（短接 2-3 是 USB_FS）。COM0 是 **RS232 电平**，请用 USB-RS232 串口线；如果用 USB-TTL，请直接接 PA9 / PA10 / GND 的 **TTL 电平**，不要接 DB9（来源：用户指南 V2.0 第 15 页原理图、第 30 页串口参数）。
- **I2C / EEPROM（默认例程需要）**：I2C0 = PB6(SCL) / PB7(SDA)，外部上拉到 3.3V，接 AT24C02（或兼容）器件，地址 `0xA0`，容量 256 字节、页大小 8 字节。**若总线上没有 AT24C02，默认例程会自检失败（打印 `24C02 test FAIL!` 且三灯常亮）**；这是预期行为，不是固件 Bug。
- **ADC 外部通道（双 ADC 模式需要）**：CH13 = PC3、CH15 = PC5（均 `GPIO_MODE_ANALOG`）。「内部通道模式」采集片内温度/Vref/Vbat，**无需外部接线**；「双 ADC 跟随/并行模式」需要把 PC3、PC5 接到待测信号（如电位器、分压），悬空会读到噪声。
- **DAC 输出（切换例程需要）**：PA4，接万用表/示波器测电压（0~3.3V 阶梯）。
- 引脚分配与代码完全一致（来源：用户指南 V2.0 表 2-1，第 8 页；按键上拉电阻见原理图 V1.0 第 6 页）：

| 功能 | 引脚 | 功能 | 引脚 |
| --- | --- | --- | --- |
| LED1 | PE2 | Wakeup 键 | PA0 |
| LED2 | PE3 | Tamper 键 | PC13 |
| LED3 | PF10 | User 键 | PB14 |
| USART0_TX | PA9 | USART0_RX | PA10 |
| ADC_CH13 | PC3 | ADC_CH15 | PC5 |
| DAC_OUT | PA4 | I2C_SCL | PB6 |
| | | I2C_SDA | PB7 |

### 3.3 获取与编译

```bash
git clone https://github.com/you-678/GD32F470IKH6.git
cd GD32F470IKH6
```

1. 用 µVision 打开 **`GD32F470IKH6/Project/F470IKH6_demo.uvprojx`**（仓库根目录和源码目录同名，是两层 `GD32F470IKH6/`，别只进第一层；Target：`GD32F470IKH6_demo`）。
2. `Project → Build Target`（F7）。产物在 `GD32F470IKH6/Project/Objects/`：`F470IKH6_demo.axf`、`F470IKH6_demo.hex`、`F470IKH6_demo.build_log.htm`（构建日志）、`F470IKH6_demo.htm`（静态调用图 / 栈用量）。**本工程未生成 `.map` 文件**。
3. `Download`（F8，或把 `.hex` 烧进 Flash）后复位运行。

> `.gitignore` 忽略了 `Objects/`、`Listings/`、`*.o`、`*.d`、`*.hex`、`*.map`，**clone 后仓库里没有固件，也没有构建日志，必须自行编译**。本地最新一次构建（2026-11-12）：`0 Error(s), 0 Warning(s)`，Code=9572 / RO-data=620 / RW-data=8 / ZI-data=1312。

### 3.4 常见报错与排错

| 现象 | 处理 |
| --- | --- |
| `#error "Please select the target GD32F4xx device in gd32f4xx.h file"` | `Firmware/CMSIS/GD/GD32F4xx/Include/gd32f4xx.h` 里的器件宏是注释状态，本机由 DFP 环境注入；换环境后报此错，在 `Options for Target → C/C++ (AC6) → Define` 添加 `GD32F470` |
| 串口无输出 | 依次查：JP5 是否短接 1-2、串口线是否接 COM0、终端是否 115200-8-N-1、是否误把 RS232 口接到 USB-TTL；默认 `main.c` 跑 I2C 自检，上电即有串口打印（见第 5 节），与按键无关 |
| 默认例程三灯常亮 / 打印 `24C02 test FAIL!` | I2C0 总线（PB6/PB7）上没接 AT24C02 或地址/上拉不对；接上器件或改用 `BSP_I2C_MODE_SW` 软件模拟（需同样接线），自检通过即循环单闪 |
| 温度/电压打印为 `0.000` | 仅当改用 **Microlib** 时才需要；请在 `Options for Target → C/C++ (AC6)` 勾选「Use float with printf」再编译。本工程用 ARMCLANG 标准库（非 Microlib），`%f` 默认可用 |
| 双 ADC 模式读数恒为 0 / 乱跳 | 内部通道模式无需接线；双 ADC 模式需 PC3、PC5 接真实模拟信号，悬空会读到噪声 |
| DAC 输出电压不变 | 切换例程未把 PA4 接到测量端；`app_dac_scan_task()` 需在主循环不断调用（每次含 10ms 延时）才产生阶梯波 |
| 中文注释乱码 | 部分源码与 `Doc/readme.txt` 是 GBK 编码，用 GBK / GB18030 打开即可，不影响编译 |
| VS Code Keil Assistant 提示「发现无效的文件组」 | 插件解析 `OSAL/`/`Services/` 等空目录告警，不影响 µVision 编译 |

---

## 4. 使用方法

### 4.1 跑通默认例程（I2C-EEPROM 两阶段自检：硬件 I2C → 软件 I2C）

1. JP5 短接 1-2，串口线接 COM0，打开串口助手：**115200 / 8 数据位 / 1 停止位 / 无校验 / 无流控**。
2. I2C 总线（PB6/PB7）接好 AT24C02（地址 0xA0，SCL/SDA 各上拉 3.3V）——硬件 I2C 与软件 I2C 复用同一组引脚/器件，只需接一份。
3. 编译（F7）→ 下载（F8）→ 复位。
4. 上电观察串口：先打印 `Stage 1: Test Hardware I2C (HW)` 横幅与硬件 I2C 自检结果，停顿 1s 后打印 `Stage 2: Test Software I2C (SW)` 横幅与软件 I2C 自检结果，最后 `Both HW & SW Tested. Entering Loop`；板上三灯随后循环单闪（约每 500ms 切换一颗）。**若没接 EEPROM，两阶段都失败，循环里三灯常亮，串口两阶段均打印 `24C02 test FAIL!`**。

### 4.2 切换例程

把下面片段替换 `User/main.c` 的 `main()` 主体即可（需要的外设 `init` 已写在片段里）。`App/`、`BSP/`、`Components/` 的 `.c` 无需增删。改完重新 Build + Download。

```c
#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"

int main(void) {
    systick_config();
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_PRINTF, 115200U);

    /* ① 流水灯：初始化三颗 LED，循环跑流水 */
    bsp_led_init(LED1); bsp_led_init(LED2); bsp_led_init(LED3);
    while(1) { app_led_flow_task(); }

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

    /* ⑦ I2C-EEPROM（即默认例程） */
    // app_i2c_eeprom_init(BSP_I2C_MODE_HW);
    // while(1) { app_i2c_eeprom_task(); }
}
```

各例程现象见第 5 节。**按键中断为上升沿触发**（按键按下为低电平、松开回到高电平），因此回调实际在**松手瞬间**触发 LED 翻转。更多完整 `main()` 示例（含 01 流水灯、03 EXTI、06 USART、09 ADC、10 DAC）见 `Doc/readme.txt`。

### 4.3 串口两种模式的真实状态

`bsp_usart_init()` 第 2 个参数：

| 模式 | 代码做了什么 | 能否当功能用 |
| --- | --- | --- |
| `BSP_USART_MODE_PRINTF` | 仅轮询发送，`printf` 重定向到 USART0；接收始终使能但无中断，需自行读寄存器 | ✅ **当前默认 main 使用**，用于打印 I2C 自检进度，**只发不收** |
| `BSP_USART_MODE_IRQ` | 使能 RBNE 中断 + NVIC；`USART0_IRQHandler`（在 `bsp_usart.c`）把字节存入 256 字节 `rx_buffer`，每收到一字节置 `rx_complete=1`；`app_usart_task()` 取数后 `printf` 回显并 `bsp_usart_send_dma()` 用 DMA 发回 | ✅ **回显链路已通**：发任意字符串回显 `[IRQ RX] Got: <原串>`；发 `BAUD9600` 切到 9600 波特率 |

> **串口号不可随便改**：`usart_typedef_enum` 只有 `BSP_USART_COM0`（其后 `BSP_USART_COMn = 1` 为计数），`bsp_usart_init()` 首行 `if (usart_num >= BSP_USART_COMn) return;`。**传其他值会静默不初始化**，只有 COM0 可用；要扩展需补齐 `bsp_usart.c` 里的查表数组。

### 4.4 常用接口速查

```c
/* LED (BSP/bsp_led.h) */
void bsp_led_init(led_typedef_enum lednum);            /* LED1/LED2/LED3 */
void bsp_led_on/off/toggle(led_typedef_enum lednum);

/* KEY (BSP/bsp_key.h) */
void bsp_key_init(key_typedef_enum k, keymode_typedef_enum mode);
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
void bsp_dac_set_value(uint16_t value);              /* 0~4095 -> 0~VDDA(约3.3V) */

/* I2C (BSP/bsp_i2c.h) */
void bsp_i2c_init(bsp_i2c_mode_enum mode);           /* BSP_I2C_MODE_HW / BSP_I2C_MODE_SW */
uint8_t bsp_i2c_write_bytes(uint8_t dev, uint8_t reg, uint8_t *p, uint16_t len);
uint8_t bsp_i2c_read_bytes(uint8_t dev, uint8_t reg, uint8_t *p, uint16_t len);
void bsp_i2c_wait_standby(uint8_t dev);

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

/* 引脚集中定义 (User/bsp_pin_def.h)：LED/KEY/USART/ADC/DAC/I2C 全在此 */
```

这些声明由 `User/gd32f4xx_libopt.h` 统一汇总（含标准外设库头、`stdio.h`/`string.h`、`systick.h`、全部 `BSP/`、`App/`、`Components/` 头与 `bsp_pin_def.h`），`gd32f4xx.h` 会自动包含它，因此各 `.c` 只要 `#include "gd32f4xx.h"` 就能拿到全部声明。

---

## 5. 输入输出示例

### 5.1 输入

| 输入 | 形式 | 代码路径 / 说明 |
| --- | --- | --- |
| 上电 / 复位 | 硬件 | `systick_config()` → `bsp_usart_init(COM0, PRINTF, 115200)` → `app_i2c_eeprom_init(HW)` → `while(1) app_i2c_eeprom_task()` |
| 代码层参数 | `User/main.c` | I2C 模式 `BSP_I2C_MODE_HW`、USART 串口号 COM0、模式 PRINTF、波特率 115200 |
| I2C 总线信号 | AT24C02 应答（SCL=PB6/SDA=PB7，上拉 3.3V） | `app_i2c_eeprom_init()` 探测并整片读写比对 |
| （切换例程后）按键 | 低电平按下 / 高电平松开 | 轮询任务 20ms 消抖；中断为**上升沿**（松手触发回调） |
| （切换例程后）串口助手下发 | 文本，以 `\r\n` 结尾最佳 | IRQ 模式下由 `app_usart_task()` 取数；含 `BAUD9600` 触发降速，否则回显 |
| （切换例程后）PC3 / PC5 模拟信号 | 电压 | 双 ADC 模式由 TIMER1 触发采样，DMA 搬运 |
| （切换例程后）PA4 测量端 | 万用表/示波器 | DAC 阶梯波输出 |

### 5.2 输出

**① 默认 `main.c` 上电后串口（EEPROM 已接好，两阶段均通过）**

```
========================================
  Stage 1: Test Hardware I2C (HW)       
========================================

The I2C is hardware interface
The speed is 100000
before at24c02_init
at24c02_init ret = 0
before at24c02_test
at24c02_test ret = 0
24C02 test OK!

========================================
  Stage 2: Test Software I2C (SW)       
========================================

The I2C is software interface
The speed is 100000
before at24c02_init
at24c02_init ret = 0
before at24c02_test
at24c02_test ret = 0
24C02 test OK!

========================================
  Both HW & SW Tested. Entering Loop    
========================================
```

> 若总线上无 AT24C02，则两阶段的 `at24c02_init ret = 1` / `at24c02_test ret = 1` / `24C02 test FAIL!`，循环里三灯**常亮**（而非循环单闪）。板上 LED 行为由最后一次（SW）测试结果决定：通过 → 每 500ms 依次亮 LED1 / LED2 / LED3；失败 → LED1/2/3 全亮。硬件 I2C 与软件 I2C 复用同一总线/器件，所以通常两者同过或同败。

**② 切换为「串口 IRQ 回显」例程后**（`app_usart_task()` 在循环中）：

```
[IRQ RX] Got: hello
（同时 DMA 把 "hello" 原样发回串口助手）

Changing baudrate to 9600...
[Success] Baudrate is now 9600 bps.
（发送 "BAUD9600" 后，后续通信需把终端切到 9600）
```

**③ 切换为「按键轮询 / 中断」例程后**：

```
[Polling] PA0 Pressed! LED1 Toggle.      // 轮询模式，按下并消抖后
[EXTI Interrupt] PC13 Pressed! LED2 Toggle. // 中断模式，松手（上升沿）瞬间
```

> 注意：中断模式回调打印发生在**松手瞬间**（上升沿），不是按下瞬间。

**④ 切换为「DAC 阶梯波」例程后**：PA4 输出 0V → 3.3V → 0V 循环扫描，万用表/示波器可见阶梯上升与下降。

### 5.3 现象对照速查（默认 `main.c` = I2C-EEPROM 自检）

| 操作 | 预期 LED | 预期串口 |
| --- | --- | --- |
| 上电复位（EEPROM 已接好） | 先全灭，HW→SW 两阶段自检后循环单闪 | 依次打印 `Stage 1 HW` → `Stage 2 SW` 自检结果 + `Both HW & SW Tested. Entering Loop` |
| 上电复位（无 EEPROM） | 两阶段均失败，三灯常亮 | 两阶段均打印 `24C02 test FAIL!` |
| 向串口发数据 | 无变化 | 无应答（默认 main 跑 PRINTF 模式，不接收） |
| 按任意键 | 无变化 | 无输出（默认 main 不读键） |

---

## 6. 目录结构

```
GD32F470IKH6/                        # 仓库根目录
├── .gitignore                      # 忽略 Objects/、Listings/、*.o/*.d/*.hex/*.map 等构建产物
├── LICENSE                         # 用户代码 MIT（Copyright (c) 2026 琪瑶luo）
├── README.md
└── GD32F470IKH6/                   # 源码根目录（与仓库根目录同名）
    ├── Project/                    # F470IKH6_demo.uvprojx / .uvoptx，构建产物 Objects/、Listings/
    ├── User/                       # main.c/.h、systick.c/.h、gd32f4xx_it.c/.h、gd32f4xx_libopt.h、bsp_pin_def.h
    ├── BSP/                        # bsp_led、bsp_key、bsp_usart、bsp_adc、bsp_dac、bsp_i2c（板级驱动）
    ├── App/                        # app_led、app_key、app_usart、app_adc、app_dac、app_i2c_eeprom（例程任务函数）
    ├── Components/                 # 组件层（芯片无关）
    │   └── drivers/                # drv_at24c02.c/.h（已落地）；protocols/、third_party/ 为预留空目录
    ├── OSAL/                       # 预留空目录（系统抽象层，待接入）
    ├── Services/                   # 预留空目录（服务层，待接入）
    ├── Firmware/                   # CMSIS + GD32F4xx 标准外设库 + USB 库源码（USB 未加入编译）
    ├── Doc/                        # 评估板原理图 PDF、用户指南 PDF、readme.txt 实验/例程文档
    └── .vscode/                    # Keil Assistant / IntelliSense 配置与日志
```

`.workbuddy/` 是本地助手的工作记录目录，与固件无关。

---

## 7. 当前状态与已知限制

- 默认 `main.c` 跑两阶段 I2C-EEPROM 自检：先 `app_i2c_eeprom_init(BSP_I2C_MODE_HW)`，延时 1s 后 `app_i2c_eeprom_init(BSP_I2C_MODE_SW)`，再进入 `while(1) app_i2c_eeprom_task()`；循环 LED 行为由最后一次（SW）测试结果决定。`app_led_*`、`app_key_*`、`app_usart_task()`、`app_adc_*`、`app_dac_scan_task()` 都不在 `main` 循环里，需按第 4.2 节手动改 `main.c`。
- 默认例程依赖 I2C0 总线上的 AT24C02 器件；板载评估板若无该 EEPROM，自检会 FAIL（三灯常亮），属预期行为。
- 按键中断为**上升沿触发**：评估板按键按下为低电平，松开回到高电平，因此回调在松手瞬间触发（见第 4.2、5.2 节）。若希望「按下即触发」，把 `bsp_key.c` 里 `exti_init(..., EXTI_TRIG_RISING)` 改成 `EXTI_TRIG_FALLING` 即可。
- `app_key_polling_task()` 用 `while(...==0)` 死等松手，长按会阻塞整个主循环；`app_led_flow_task()` 为阻塞延时，约 3 秒跑完一轮。
- 双 ADC 模式需要 PC3 / PC5 接真实模拟信号；`app_adc_task_dual_dma()` 打印的是 32 位同步数据寄存器原始值，需按 GD32 手册拆分两路 ADC 的 16 位数据。
- DAC 为 12 位右对齐，输出 0~VDDA（约 3.3V）；`app_dac_scan_task()` 每 100ms 步进 100，需主循环不断调用。
- `BSP_USART_COMn = 1`，只有 `BSP_USART_COM0` 可用；`usart_typedef_enum` 的其他值会静默返回。
- I2C 支持硬件（I2C0）与软件（GPIO 模拟）两套实现，均带超时保护；软件模拟用 `gpio_bit_*` 翻转 PB6/PB7。
- 旧版「独立 DMA 接收模式」（`BSP_USART_MODE_DMA` 枚举与 `bsp_usart_dma_rx/tx_init`）已移除；串口 DMA 现以 `bsp_usart_send_dma()`（发送）+ IRQ 接收 + `bsp_usart_get_rx_data()`（取数）的形式提供，回显已验证可用。
- 部分源码与 `Doc/readme.txt` 的中文注释为 GBK 编码，UTF-8 编辑器直接打开会乱码（不影响编译）。
- `Firmware/` 下的 CMSIS、标准外设库、USB 库版权归 GigaDevice Semiconductor Inc.，遵循其文件头中的 BSD-3-Clause 条款；本仓库自有 `User/`、`BSP/`、`App/`、`Components/` 代码按根目录 `LICENSE` 的 MIT 许可使用。

---

## 8. 如何复核本 README（一致性依据）

| 声明 | 依据文件 |
| --- | --- |
| 默认 `main.c` 行为（I2C-EEPROM 自检、LED 状态指示、无 KEY/ADC/DAC 调用） | `User/main.c`、`App/app_i2c_eeprom.c`、`BSP/bsp_dac.c`(未调用) |
| 引脚/时钟集中定义（LED/KEY/USART/ADC/DAC/I2C） | `User/bsp_pin_def.h` |
| 按键回调、上升沿中断、`bsp_key_irq_handler` 分发 | `BSP/bsp_key.c/.h`、`User/gd32f4xx_it.c` |
| USART 两模式、256 字节接收缓冲、`set_baudrate`/`send_dma`/`get_rx_data`、IRQ 回显 | `BSP/bsp_usart.c/.h`、`App/app_usart.c` |
| DAC 接口（PA4、12 位、阶梯波任务） | `BSP/bsp_dac.c/.h`、`App/app_dac.c` |
| I2C 硬/软双实现、AT24C02 组件驱动 | `BSP/bsp_i2c.c/.h`、`Components/drivers/drv_at24c02.c/.h` |
| ADC 三模式、TIMER1 触发、DMA1_CH0、温度/电压换算 | `BSP/bsp_adc.c/.h`、`App/app_adc.c` |
| LED/KEY/USART/ADC/DAC/I2C 的 App 任务函数与打印字符串 | `App/*.c` |
| EXTI / USART0 中断服务位置、SysTick | `User/gd32f4xx_it.c`、`User/systick.c` |
| 器件、DFP 版本、AC6/C99/-O2、文件组、USB 库未编译 | `Project/F470IKH6_demo.uvprojx` |
| 编译器版本、0 Error 0 Warning、Code/RO/RW/ZI（9572/620/8/1312） | `Project/Objects/F470IKH6_demo.build_log.htm`（本机文件，未入库，2026-11-12 14:38） |
| 引脚分配、K2/K3/K4 编号、JP5 跳线、MAX3232/COM0、115200 参数 | `Doc/GD32470I-EVAL评估板用户指南_V2.0.pdf`（表 2-1、第 15/30 页）、`Doc/GD32F470I-EVAL-V1.0.pdf`（第 6 页按键上拉） |
| 01–29 例程框架与完整 main 调用片段 | `Doc/readme.txt`（GBK） |
| clone 地址 | `git remote -v`（`https://github.com/you-678/GD32F470IKH6.git`） |
