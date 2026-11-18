# v0.1.0 详细发布说明（本地核对归档）

> 本文件是 `RELEASE_NOTES.md` 的详细版，用于本地归档与发布前核对；**GitHub Release 正文以 `RELEASE_NOTES.md` 为准**。
>
> 📌 **版本说明（2026-11-18 更新）**：v0.1.0 tag 已强推到当前 TRNG 版本（`9544e11`）。原 11-13 的「全外设串联自检 Harness」默认程序不再随 tag 交付，其完整代码与日志保留在 `Doc/readme.txt` 第三章作历史参考。本文 §1–§7 描述**当前 tag 对应功能**，§8 保留 Harness 历史快照。

## 1. Tag 与 Release 参数

| 项 | 值 |
| --- | --- |
| Tag | `v0.1.0`（强推前移到 `9544e11`，2026-11-18） |
| 推送分支 | `main` |
| 仓库 | `you-678/GD32F470IKH6`，public |
| Release 标题 | `v0.1.0 · TRNG 真随机数 + 全模块集成（EXMC 深睡 / SPI·I2S 双模 / SDIO / CAN / OV2640）` |
| Release 资产 | `F470IKH6_demo.hex`（**2026-11-18 按当前 TRNG 源码重编**，28,025 字节，SHA256 `0b5b1505…cb5cb5`） |
| 文档基准 | `README.md` / `Doc/readme.txt` 2026-11-18 版 |

## 2. 简介的发布说明（短版，用于 Release 摘要 / 社交分享）

> GD32F470IKH6 裸机学习工程首个版本。把官方分散 Demo 整合成 `BSP / App / User / Components` 分层的单工程，默认跑 TRNG 硬件随机数测试（串口命令行 `TRNG` / `BAUD9600`），同时编译集成 LED/按键、ADC/DAC、I2C/SPI/I2S、EXMC 深睡唤醒、SDIO、CAN、RTC/PMU/CTC、DCI 摄像头与 TLI 显示等 14 类外设模块——切换例程只需改 `main()`。串口 115200 直接输出日志。

## 3. 项目解决什么问题

| 痛点 | 本仓库做法 |
| --- | --- |
| 官方例程「一个 Demo 一个工程」，对比各外设写法要反复建 / 切工程 | 外设抽成 `BSP/` 板级驱动、业务逻辑抽成 `App/` 任务函数、协议抽成 `Components/drivers/`，**只改 `User/main.c` 就能切换例程** |
| 裸机调试只能点灯猜状态 | `printf` 重定向 USART0（PA9/PA10，115200-8-N-1），含浮点输出；默认 TRNG 例程用串口命令行触发 |
| 换板卡要改一堆散落引脚宏 | 引脚 / 时钟 / AF / DMA 宏集中在 `User/bsp_pin_def.h`，驱动层与业务层统一取用 |
| 代码写了、现象没记 | `Doc/readme.txt` 保留分层架构、29 例程总览与多份完整 `main()` 调用示例 |

## 4. 当前默认例程：TRNG 硬件随机数测试

当前 `User/main.c`（commit `9544e11`）只有约 51 行，已无 `RUN_xx` / `AUTO_RUN_ALL` 开关：

```c
int main(void)
{
    bsp_usart_init(BSP_USART_COM0, BSP_USART_MODE_IRQ, 115200U);
    printf("\r\n /==============Gigadevice TRNG test=============/ \r\n");
    if(ERROR == bsp_trng_init()) { printf("TRNG init error \r\n"); while(1); }
    printf("TRNG init ok \r\n");
    printf("Send 'TRNG' to generate random number.\r\n");
    while(1) { app_usart_task(); }
}
```

串口命令：`TRNG` → 两个 0~255 硬件随机数；`BAUD9600` → 运行时切 9600；其它字符串 → `[IRQ RX] Got: <原串>` 并 DMA 回显。

> ⚠️ 默认 `main` 未调用 `systick_config()` / `bsp_led_init()` / `bsp_key_init()`，上电不点灯、按键无响应是真实代码行为；`app_usart_task()` 里虽有 `bsp_led_toggle(LED1)`，但 LED GPIO 未初始化，通常看不到变化。

## 5. 当前交付范围（tag `9544e11`）

### 5.1 默认 `main()` 实际调用

| 模块 | 真实实现 | 关键文件 |
| --- | --- | --- |
| USART0 IRQ | `BSP_USART_MODE_IRQ`，256B 接收缓冲，`fputc` 重定向 `printf` | `BSP/bsp_usart.c/.h` |
| TRNG | 使能 `RCU_TRNG` → `trng_deinit/enable` → 等 `DRDY`（重试 3 次）；32 位真随机数与区间取值 | `BSP/bsp_trng.c/.h` |
| 串口命令行 | `app_usart_task()` 按 `\r`/`\n` 行断帧：TRNG / BAUD9600 / 回显 | `App/app_usart.c` |

### 5.2 已编译进工程、需手动切 `main()` 的模块

| 类别 | 模块 | 关键文件 |
| --- | --- | --- |
| 基础外设 | LED（PE2/PE3/PF10）、KEY（PA0/PC13/PB14，轮询 + EXTI） | `bsp_led.*`、`bsp_key.*`、`app_led.*`、`app_key.*` |
| 模拟 | ADC 三模式、DAC PA4 12 位阶梯波 | `bsp_adc.*`、`bsp_dac.*`、`app_adc.*`、`app_dac.*` |
| 串行总线 | I2C0 + AT24C02（硬 / 软双模）、SPI5 + GD25Qxx（Quad，SW 降级） | `bsp_i2c.*`、`bsp_spi.*`、`drv_at24c02.*`、`drv_gd25qxx.*`、`app_i2c_eeprom.*` |
| 音频 | I2S1 硬 / 软双模 + WAV Player（`wavetestdata` 8000Hz / mono / 16bit） | `bsp_i2s.*`、`drv_wav.*`、`drv_i2s_player.*`、`wave_data.*` |
| 存储 | EXMC SDRAM 32MB @ `0xC0000000`（自刷新 + 深睡唤醒）、NAND @ `0x70000000`、SDIO SD 卡 | `bsp_exmc.*`、`drv_storage.*`、`bsp_sdio.*`、`drv_sdcard.*`、`app_sdcard.*` |
| 通信 / 时钟 | CAN0（PB9/PB8 AF9）、Clock-out、CTC、RTC、PMU、TIMER PWM | `bsp_can.*`、`app_can.*`、`bsp_clockout.*`、`bsp_ctc.*`、`bsp_rtc.*`、`bsp_pmu.*`、`bsp_timer.*`、`app_*.c` |
| 显示 / 图像 | TLI + IPA + LCD（12 张 `image*.h`）、DCI + OV2640（SCCB + DMA1_CH7） | `bsp_tli.*`、`bsp_ipa.*`、`app_lcd.*`、`bsp_dci.*`、`bsp_sccb.*`、`OV2640/dci_ov2640.*`、`app_camera.*` |

**工程规模**：BSP 21 组、App 15 组、`gd32f4xx_*.c` 31 个参与编译（新增 can/ctc/dci/ipa/rtc/sdio/tli/trng）；新增 `SDIO_IRQHandler`、`DMA1_Channel7_IRQHandler`。

## 6. 工程配置与验证环境

- Keil µVision V5.40.0.0 / MDK-ARM Plus 5.40.0.3 / **ARMCLANG V6.19（AC6，C99，-O2）**
- `GigaDevice.GD32F4xx_DFP 3.2.0`；器件宏 `GD32F470`
- 工程：`GD32F470IKH6/Project/F470IKH6_demo.uvprojx`，Target `GD32F470IKH6_demo`
- `.gitignore` 忽略 `Objects/`、`Listings/`、`*.o`、`*.d`、`*.hex`、`*.map`；仓库不含固件，须本地编译
- 许可：自有代码 MIT（`LICENSE`）；`Firmware/` 下 GigaDevice 官方库遵循其文件头 BSD-3-Clause
- 编码：`Doc/readme.txt` 与根目录 `.md` 为 UTF-8；部分源码中文注释为 GBK（不影响编译）

## 7. 已知限制

- 默认 `main` 无 LED / KEY / SysTick 初始化，上电不点灯、按键无响应（真实行为）。
- 29 例程中 `27_ENET`、`28/29_USB` 为标题占位；USB 库未加入编译；`OSAL/`、`Services/`、`Components/protocols`、`Components/third_party` 为预留空目录。
- `BSP/`、`App/` 下 `*.bak_*` 调试备份不参与编译。
- Release 附件 `F470IKH6_demo.hex` 已于 2026-11-18 **按当前 TRNG 源码重新编译替换**（28,025 字节，SHA256 `0b5b1505…cb5cb5`），与 tag 源码一致。
- 编码混用：源码部分 GBK、文档 UTF-8，按对应编码打开。

---

## 8. 历史快照：初版 Harness 默认例程（2026-11-13，已不随 tag 交付）

> 以下内容为 v0.1.0 初版（commit `bf6d784` 及之前）的默认程序与 11-13 发布时的实测基线。**当前 tag 已强推到 `9544e11`（TRNG 版）**，此段仅作历史归档。

初版 `User/main.c`（commit `bf6d784` 之前）用编译开关选例程，`AUTO_RUN_ALL=1` 时每项跑固定时长后自动进入下一项（`TEST_END()` = `delay_1ms(1000)`），`=0` 时停在 `while(1)` 单测：

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

上电依次：① LED → ② KEY（轮询 + EXTI）→ ③ USART（心跳 + IRQ / DMA 回显）→ ④ ADC（内部温度 / Vref / Vbat + 双 ADC DMA）→ ⑤ DAC 阶梯波 → ⑥ SPI Flash（硬 / 软 Quad 比对）→ ⑦ I2S（硬 / 软播放 WAV）→ ⑧ EXMC（SDRAM / 深睡唤醒 / NAND），最后 `ALL TESTS COMPLETED SUCCESSFULLY!`。完整代码与日志归档于 `Doc/readme.txt` 第三章（标题已标「历史参考」）。

**初版实测基线**：ADC 温度 25℃ / Vref 1.217V / Vbat 3.323V；硬件 SPI `quad=yes, cap=2048KB`、软件 SPI `quad=no`；I2S `rate=8000 ch=1 bits=16`；深睡唤醒 `Woke up! Clock restored.`；NAND ID `0xC8 0xF1 0x80 0x1D`。

**初版 → 当前的变化**：

| 项 | 初版（11-13） | 当前 tag（11-18） |
| --- | --- | --- |
| 默认例程 | 全外设串联 Harness | TRNG 硬件随机数测试 |
| 串口模式 | PRINTF（内部切 IRQ） | `BSP_USART_MODE_IRQ` 全程 |
| 断帧 | 收 8 字节即返回（分包） | 按 `\r`/`\n` 整行断帧 |
| 上电 LED / KEY | 初始化并翻转 | 未初始化，不点灯 / 按键无响应 |
| 标准库 `.c` 数 | 27 个 | 31 个 |
| 新增模块 | — | SDIO / CAN / Clock-out / CTC / RTC / PMU / TIMER / TLI / IPA / DCI+OV2640 / TRNG |
