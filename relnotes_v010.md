# v0.1.0 详细发布说明（本地归档）

> 本文件是 `RELEASE_NOTES.md` 的详细版，用于本地归档与发布前核对；**GitHub Release 正文以 `RELEASE_NOTES.md` 为准**。
> 内容逐条对照当前工作区源码核实（2026-11-13），与 `README.md`、`Doc/readme.txt` 保持一致。

## 1. Tag 与 Release 参数

| 项 | 值 |
| --- | --- |
| Tag | `v0.1.0` |
| 推送分支 | `main` |
| 仓库可见性 | public |
| Release 标题 | `v0.1.0 — 全外设串联自检 Harness（EXMC 深睡唤醒 / I2S·SPI 双模 / GD32F470IKH6）` |
| Release 资产 | `F470IKH6_demo.hex`（早前构建产物，建议按当前源码重编） |

## 2. 简介的发布说明（短版，用于 Release 摘要 / 社交分享）

> GD32F470IKH6 裸机学习工程首个版本。把官方分散 Demo 整合成 `BSP / App / User / Components` 分层的单工程，默认程序一键串联自检 LED、按键、串口、ADC、DAC、SPI Flash、I2S、EXMC 八类外设；实现 SDRAM 深睡唤醒数据保持、原生 `__WFI()` 低功耗、SPI/I2S 硬件与软件双模切换。串口 115200 直接输出全量自检日志。

## 3. 项目解决什么问题

| 痛点 | 本仓库做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，对比各外设写法要反复建 / 切工程 | 外设抽成 `BSP/` 板级驱动、例程逻辑抽成 `App/` 任务函数，**只改 `User/main.c` 就能切换例程** |
| 裸机调试只能点灯猜状态 | `printf` 重定向 USART0（PA9/PA10，115200-8-N-1），含浮点输出，直接看日志 |
| 换板卡要改一堆散落引脚宏 | 引脚 / 时钟宏集中在 `User/bsp_pin_def.h`，驱动层与业务层统一取用 |
| 代码写了、现象没记 | `Doc/readme.txt` 保留分层架构、例程总览与完整 `main()` 调用示例 |

## 4. 默认例程：全外设串联自检 Harness

`User/main.c` 顶部用编译开关选择例程，`AUTO_RUN_ALL=1` 时每项跑固定时长后自动进入下一项（`TEST_END()` = `delay_1ms(1000)`），`=0` 时每项停在 `while(1)` 便于单测：

```c
#define AUTO_RUN_ALL   1
#define RUN_01_LED     1
#define RUN_03_KEY     1
#define RUN_06_USART   1
#define RUN_09_ADC     1
#define RUN_10_DAC     1
#define RUN_12_SPI     1
#define RUN_13_I2S     1
#define RUN_EXMC       1
```

上电后依次：① LED 全功能 → ② KEY（轮询 15s + EXTI 中断）→ ③ USART（Printf 心跳 + IRQ 回显 + DMA 回显）→ ④ ADC（内部温度 / Vref / Vbat + 双 ADC DMA 跟随 / 并行）→ ⑤ DAC 阶梯波 → ⑥ SPI Flash（硬 / 软双模 Quad 比对）→ ⑦ I2S（硬 / 软双模播放 WAV）→ ⑧ EXMC（SDRAM / 深睡唤醒 / NAND）→ 打印 `ALL TESTS COMPLETED SUCCESSFULLY!`。完整实测串口日志见 README §5.2。

## 5. 当前功能（v0.1.0 交付范围）

| 模块 | 真实实现 | 默认 Harness |
| --- | --- | --- |
| LED | 3 路 PE2/PE3/PF10；`init/on/off/toggle` + `app_led_all_on/off/toggle`、`app_led_flash(n)`、`app_led_flow_task()` | ✅ `Test_01_LED` |
| KEY | 3 键 PA0/PC13/PB14；轮询 + EXTI 中断回调、`bsp_key_register_callback()`、`bsp_key_set_trigger()`（默认上拉下降沿） | ✅ `Test_03_KEY` |
| USART0 | PA9/PA10 AF7；`printf` 重定向；PRINTF / IRQ 两模式；`set_baudrate()`、`send_dma()`、`get_rx_data()`，256B 接收缓冲 | ✅ 全程打印 + `Test_06_USART` |
| ADC | 三模式：内部通道（温度 / Vref / Vbat）、双 ADC 跟随 DMA、双 ADC 并行 DMA（TIMER1 触发，DMA1_CH0） | ✅ `Test_09_ADC` |
| DAC | PA4，12 位右对齐，`app_dac_scan_task()` 阶梯波 0V→3.3V→0V | ✅ `Test_10_DAC` |
| I2C + AT24C02 | 硬件 I2C0 + 软件模拟双实现（带超时）；AT24C02 单字节 / 分页写 / 连续读 / 整片一致性测试 | ❌ 默认未调用，需按 README §4.2 手动接入 |
| SPI + GD25Qxx | SPI5 硬件 + GPIO 模拟双模；Quad 进出 / 读写，ID 与容量探测、512B 擦写读比对；SW 模式自动降级单线 | ✅ `Test_12_SPI` |
| I2S + WAV | I2S1 硬件 + 软件双模；`drv_wav` 解析、`drv_i2s_player` 播放嵌入 `wavetestdata`（8000Hz / mono / 16bit） | ✅ `Test_13_I2S` |
| EXMC / Storage | SDRAM（32MB @ 0xC0000000，自刷新 + 深睡唤醒时钟恢复）、NAND（@ 0x70000000，读 ID / 擦写读）；`drv_storage_run()` 三模式 | ✅ `Test_EXMC` |
| 系统 / 中断 | SysTick 1kHz → `delay_1ms()`；EXTI0 / EXTI10_15 分发按键；USART0 / SPI1 中断下沉到 BSP | ✅ |
| 官方固件 | CMSIS（25MHz HXTAL → PLL 200MHz）+ GD32F4xx 标准外设库（27 个 `.c`）全部参与编译 | ✅ |

**实测基线（GD32470I-EVAL）**：ADC 温度 25℃ / Vref 1.217V / Vbat 3.323V；硬件 SPI `quad=yes, cap=2048KB`、软件 SPI `quad=no`；I2S `rate=8000 ch=1 bits=16`；深睡唤醒 `Woke up! Clock restored.`；NAND ID `0xC8 0xF1 0x80 0x1D`。

## 6. 工程配置

- Keil µVision V5.40 / MDK-ARM 5.40 / **ARMCLANG V6.19（AC6，C99，-O2）**
- `GigaDevice.GD32F4xx_DFP 3.2.0`；器件宏 `GD32F470`
- 工程：`GD32F470IKH6/Project/F470IKH6_demo.uvprojx`，Target `GD32F470IKH6_demo`
- `.gitignore` 忽略 `Objects/`、`Listings/`、`*.o`、`*.d`、`*.hex`、`*.map`；仓库不含固件，须本地编译
- 许可：自有代码 MIT（`LICENSE`）；`Firmware/` 下 GigaDevice 官方库遵循其文件头 BSD-3-Clause

## 7. 已知限制

- 默认 Harness 需在 GD32470I-EVAL 实板上跑满；无板载 SDRAM / NAND 或焊接异常时对应存储项打印错误但程序继续，属预期行为。
- I2C / AT24C02 有完整驱动与 App 自检，但**默认 Harness 未接入**，须手动改 `main.c`。
- `OSAL/`、`Services/`、`Components/protocols`、`Components/third_party` 为预留空目录；`Firmware/GD32F4xx_usb_library/` 未加入编译（`uvprojx` 引用数为 0），当前仅 COM0 可用。
- Release 附带的 `F470IKH6_demo.hex` 为早前提交构建产物，与最新 Harness 源码不一定一致，建议本地重编。
- 部分源码与 `Doc/readme.txt` 中文注释为 GBK 编码，UTF-8 编辑器直接打开会乱码（不影响编译）。
