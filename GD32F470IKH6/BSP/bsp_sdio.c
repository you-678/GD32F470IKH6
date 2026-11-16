#include "bsp_sdio.h"

#define SDIO_FIFO_ADDR   ((uint32_t)0x40012C80)

/*!
    \brief      SDIO 控制器初始化（时钟 + GPIO + 复位）
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void bsp_sdio_init(void)
{
    rcu_periph_clock_enable(RCU_SDIO);
    rcu_periph_clock_enable(RCU_DMA1);
    rcu_periph_clock_enable(SDIO_GPIO_CLK);
    rcu_periph_clock_enable(SDIO_GPIO_CLK_CMD);

    /* CLK */
    gpio_af_set(SDIO_CLK_PORT, SDIO_GPIO_AF, SDIO_CLK_PIN);
    gpio_mode_set(SDIO_CLK_PORT, GPIO_MODE_AF, SDIO_GPIO_PUPD, SDIO_CLK_PIN);
    gpio_output_options_set(SDIO_CLK_PORT, GPIO_OTYPE_PP, SDIO_GPIO_SPEED, SDIO_CLK_PIN);

    /* CMD */
    gpio_af_set(SDIO_CMD_PORT, SDIO_GPIO_AF, SDIO_CMD_PIN);
    gpio_mode_set(SDIO_CMD_PORT, GPIO_MODE_AF, SDIO_GPIO_PUPD, SDIO_CMD_PIN);
    gpio_output_options_set(SDIO_CMD_PORT, GPIO_OTYPE_PP, SDIO_GPIO_SPEED, SDIO_CMD_PIN);

    /* D0~D3 */
    uint32_t d_pins = SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN;
    gpio_af_set(SDIO_D0_PORT, SDIO_GPIO_AF, d_pins);
    gpio_mode_set(SDIO_D0_PORT, GPIO_MODE_AF, SDIO_GPIO_PUPD, d_pins);
    gpio_output_options_set(SDIO_D0_PORT, GPIO_OTYPE_PP, SDIO_GPIO_SPEED, d_pins);

    /* ★ 新增：DAT3 上电强制拉高，防止卡误入 SPI 模式（GD32 常见坑） */
    gpio_mode_set(SDIO_D3_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SDIO_D3_PIN);
    gpio_bit_set(SDIO_D3_PORT, SDIO_D3_PIN);
    for(volatile uint32_t i = 0; i < 0xFFFF; i++);
    gpio_af_set(SDIO_D3_PORT, SDIO_GPIO_AF, SDIO_D3_PIN);
    gpio_mode_set(SDIO_D3_PORT, GPIO_MODE_AF, SDIO_GPIO_PUPD, SDIO_D3_PIN);
    /* ★ 新增结束 */

    sdio_deinit();

    sdio_deinit();
}

/*!
    \brief      配置 SDIO 时钟分频与总线模式
    \param[in]  clkdiv: 时钟分频值
    \param[in]  busmode: SDIO_BUSMODE_1BIT / SDIO_BUSMODE_4BIT
    \param[out] 无
    \retval     无
*/
void bsp_sdio_clock_set(uint16_t clkdiv, uint32_t busmode)
{
    sdio_clock_config(SDIO_SDIOCLKEDGE_RISING, SDIO_CLOCKBYPASS_DISABLE,
                      SDIO_CLOCKPWRSAVE_DISABLE, clkdiv);
    sdio_bus_mode_set(busmode);
    sdio_hardware_clock_disable();
}

/*!
    \brief      发送一条 SDIO 命令
    \param[in]  cmd_index: 命令号
    \param[in]  arg: 命令参数
    \param[in]  resp: 响应类型
    \param[out] 无
    \retval     无
*/
void bsp_sdio_send_cmd(uint8_t cmd_index, uint32_t arg, bsp_sdio_resp_typedef_enum resp)
{
    uint32_t resp_type;
    switch(resp) {
    case BSP_SDIO_RESP_SHORT: resp_type = SDIO_RESPONSETYPE_SHORT; break;
    case BSP_SDIO_RESP_LONG:  resp_type = SDIO_RESPONSETYPE_LONG;  break;
    default:                  resp_type = SDIO_RESPONSETYPE_NO;    break;
    }
    sdio_command_response_config(cmd_index, arg, resp_type);
    sdio_wait_type_set(SDIO_WAITTYPE_NO);
    sdio_csm_enable();
}

/*!
    \brief      读取响应寄存器
    \param[in]  index: 0~3
    \param[out] 无
    \retval     响应值
*/
uint32_t bsp_sdio_get_response(uint8_t index)
{
    switch(index) {
    case 0:  return sdio_response_get(SDIO_RESPONSE0);
    case 1:  return sdio_response_get(SDIO_RESPONSE1);
    case 2:  return sdio_response_get(SDIO_RESPONSE2);
    default: return sdio_response_get(SDIO_RESPONSE3);
    }
}

/*!
    \brief      读取最近一次响应的命令号
    \param[in]  无
    \param[out] 无
    \retval     命令号
*/
uint8_t bsp_sdio_get_resp_cmd_index(void)
{
    return (uint8_t)sdio_command_index_get();
}

uint32_t bsp_sdio_read_word(void)        { return sdio_data_read(); }
void     bsp_sdio_write_word(uint32_t w) { sdio_data_write(w); }

/*!
    \brief      DMA 通用配置
    \param[in]  buf: 内存缓冲区
    \param[in]  bytes: 数据字节数
    \param[in]  dir: DMA_MEMORY_TO_PERIPH / DMA_PERIPH_TO_MEMORY
    \param[out] 无
    \retval     无
*/
static void bsp_sdio_dma_common(uint32_t *buf, uint32_t bytes, uint32_t dir)
{
    dma_single_data_parameter_struct dma_init_struct;
    dma_deinit(SDIO_DMA_PERIPH, SDIO_DMA_CHANNEL);
    dma_init_struct.periph_addr         = SDIO_FIFO_ADDR;
    dma_init_struct.memory0_addr        = (uint32_t)buf;
    dma_init_struct.direction           = dir;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_32BIT;
    dma_init_struct.priority            = DMA_PRIORITY_HIGH;
    dma_init_struct.number              = bytes / 4;
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.circular_mode       = DMA_CIRCULAR_MODE_DISABLE;
    dma_single_data_mode_init(SDIO_DMA_PERIPH, SDIO_DMA_CHANNEL, &dma_init_struct);
    dma_channel_subperipheral_select(SDIO_DMA_PERIPH, SDIO_DMA_CHANNEL, SDIO_DMA_SUBPERI);
    dma_channel_enable(SDIO_DMA_PERIPH, SDIO_DMA_CHANNEL);
}

void bsp_sdio_dma_tx_config(uint32_t *src, uint32_t bytes)
{
    bsp_sdio_dma_common(src, bytes, DMA_MEMORY_TO_PERIPH);
}

void bsp_sdio_dma_rx_config(uint32_t *dst, uint32_t bytes)
{
    bsp_sdio_dma_common(dst, bytes, DMA_PERIPH_TO_MEMORY);
}

uint8_t bsp_sdio_dma_is_done(void)
{
    return (RESET != dma_flag_get(SDIO_DMA_PERIPH, SDIO_DMA_CHANNEL, SDIO_DMA_FLAG_FTF)) ? 1 : 0;
}