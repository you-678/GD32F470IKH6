#include "bsp_tli.h"
#include "logo.h"

#define HORIZONTAL_SYNCHRONOUS_PULSE  41
#define HORIZONTAL_BACK_PORCH         2
#define ACTIVE_WIDTH                  480
#define HORIZONTAL_FRONT_PORCH        2

#define VERTICAL_SYNCHRONOUS_PULSE    10
#define VERTICAL_BACK_PORCH           2
#define ACTIVE_HEIGHT                 272
#define VERTICAL_FRONT_PORCH          2

static void tli_gpio_config(void);

void bsp_tli_init(void)
{
    tli_parameter_struct tli_init_struct;
    tli_layer_parameter_struct tli_layer_init_struct;

    /* 1. 引脚初始化 */
    tli_gpio_config();

    /* 2. 使能 TLI 时钟 */
    rcu_periph_clock_enable(RCU_TLI);

    /* 3. 配置 PLLSAI 时钟生成 LCD 像素时钟 */
    if(ERROR == rcu_pllsai_config(192, 2, 3)) {
        while(1);
    }
    rcu_tli_clock_div_config(RCU_PLLSAIR_DIV8);
    rcu_osci_on(RCU_PLLSAI_CK);
    if(ERROR == rcu_osci_stab_wait(RCU_PLLSAI_CK)) {
        while(1);
    }

    /* 4. TLI 全局时序参数配置 */
    tli_init_struct.signalpolarity_hs = TLI_HSYN_ACTLIVE_LOW;
    tli_init_struct.signalpolarity_vs = TLI_VSYN_ACTLIVE_LOW;
    tli_init_struct.signalpolarity_de = TLI_DE_ACTLIVE_LOW;
    tli_init_struct.signalpolarity_pixelck = TLI_PIXEL_CLOCK_TLI;

    tli_init_struct.synpsz_hpsz = HORIZONTAL_SYNCHRONOUS_PULSE - 1;
    tli_init_struct.synpsz_vpsz = VERTICAL_SYNCHRONOUS_PULSE - 1;
    tli_init_struct.backpsz_hbpsz = HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH - 1;
    tli_init_struct.backpsz_vbpsz = VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH - 1;
    tli_init_struct.activesz_hasz = HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH + ACTIVE_WIDTH - 1;
    tli_init_struct.activesz_vasz = VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH + ACTIVE_HEIGHT - 1;
    tli_init_struct.totalsz_htsz = HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH + ACTIVE_WIDTH + HORIZONTAL_FRONT_PORCH - 1;
    tli_init_struct.totalsz_vtsz = VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH + ACTIVE_HEIGHT + VERTICAL_FRONT_PORCH - 1;
    
    tli_init_struct.backcolor_red = 0xFF;
    tli_init_struct.backcolor_green = 0xFF;
    tli_init_struct.backcolor_blue = 0xFF;
    tli_init(&tli_init_struct);

    /* 5. Layer 0 配置 (显示 LOGO 背景) */
    tli_layer_init_struct.layer_window_leftpos = 80 + HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH;
    tli_layer_init_struct.layer_window_rightpos = (80 + 320 + HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH - 1);
    tli_layer_init_struct.layer_window_toppos = 150 + VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH;
    tli_layer_init_struct.layer_window_bottompos = (150 + 100 + VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH - 1);
    tli_layer_init_struct.layer_ppf = LAYER_PPF_RGB565;
    tli_layer_init_struct.layer_sa = 0xFF;
    tli_layer_init_struct.layer_default_blue = 0xFF;
    tli_layer_init_struct.layer_default_green = 0xFF;
    tli_layer_init_struct.layer_default_red = 0xFF;
    tli_layer_init_struct.layer_default_alpha = 0x0;
    tli_layer_init_struct.layer_acf1 = LAYER_ACF1_PASA;
    tli_layer_init_struct.layer_acf2 = LAYER_ACF2_PASA;
    tli_layer_init_struct.layer_frame_bufaddr = (uint32_t)gImage_logo;
    tli_layer_init_struct.layer_frame_line_length = ((320 * 2) + 3);
    tli_layer_init_struct.layer_frame_buf_stride_offset = (320 * 2);
    tli_layer_init_struct.layer_frame_total_line_number = 100;
    tli_layer_init(LAYER0, &tli_layer_init_struct);
}

/*!
    \brief      Layer 1 (混合层) 配置
*/
void bsp_tli_blend_layer1_init(uint32_t mem_addr, uint16_t width, uint16_t height)
{
    tli_layer_parameter_struct tli_layer_init_struct;

    tli_layer_init_struct.layer_window_leftpos = 80 + HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH;
    tli_layer_init_struct.layer_window_rightpos = (80 + width + HORIZONTAL_SYNCHRONOUS_PULSE + HORIZONTAL_BACK_PORCH - 1);
    tli_layer_init_struct.layer_window_toppos = 20 + VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH;
    tli_layer_init_struct.layer_window_bottompos = 20 + height + VERTICAL_SYNCHRONOUS_PULSE + VERTICAL_BACK_PORCH - 1;
    tli_layer_init_struct.layer_ppf = LAYER_PPF_RGB565;
    tli_layer_init_struct.layer_sa = 0xFF;
    tli_layer_init_struct.layer_acf1 = LAYER_ACF1_PASA;
    tli_layer_init_struct.layer_acf2 = LAYER_ACF2_PASA;
    tli_layer_init_struct.layer_default_alpha = 0;
    tli_layer_init_struct.layer_default_blue = 0;
    tli_layer_init_struct.layer_default_green = 0;
    tli_layer_init_struct.layer_default_red = 0;
    tli_layer_init_struct.layer_frame_bufaddr = mem_addr;
    tli_layer_init_struct.layer_frame_line_length = ((width * 2) + 3);
    tli_layer_init_struct.layer_frame_buf_stride_offset = (width * 2);
    tli_layer_init_struct.layer_frame_total_line_number = height;
    tli_layer_init(LAYER1, &tli_layer_init_struct);
}

/*!
    \brief      LCD 屏幕引脚初始化 (使用宏定义)
*/
static void tli_gpio_config(void)
{
    /* 1. 使能所有相关 GPIO 时钟 */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_GPIOH);
    rcu_periph_clock_enable(RCU_GPIOI);
    rcu_periph_clock_enable(RCU_GPIOG);
    rcu_periph_clock_enable(RCU_GPIOF);

    /* 2. 配置复用功能 AF14 */
    /* HSYNC, VSYNC, PCLK */
    gpio_af_set(LCD_HSYNC_PORT, LCD_TLI_AF, LCD_HSYNC_PIN);
    gpio_af_set(LCD_VSYNC_PORT, LCD_TLI_AF, LCD_VSYNC_PIN);
    gpio_af_set(LCD_PCLK_PORT,  LCD_TLI_AF, LCD_PCLK_PIN);

    /* R0 ~ R7 */
    gpio_af_set(LCD_R0_PORT, LCD_TLI_AF, LCD_R0_PIN);
    gpio_af_set(LCD_R1_PORT, LCD_TLI_AF, LCD_R1_PIN);
    gpio_af_set(LCD_R2_PORT, LCD_TLI_AF, LCD_R2_PIN);
    gpio_af_set(LCD_R3_PORT, LCD_TLI_AF, LCD_R3_PIN);
    gpio_af_set(LCD_R4_PORT, LCD_TLI_AF, LCD_R4_PIN);
    gpio_af_set(LCD_R5_PORT, LCD_TLI_AF, LCD_R5_PIN);
    gpio_af_set(LCD_R6_PORT, LCD_TLI_AF, LCD_R6_PIN);
    gpio_af_set(LCD_R7_PORT, LCD_TLI_AF, LCD_R7_PIN);

    /* G0 ~ G7 */
    gpio_af_set(LCD_G0_PORT, LCD_TLI_AF, LCD_G0_PIN);
    gpio_af_set(LCD_G1_PORT, LCD_TLI_AF, LCD_G1_PIN);
    gpio_af_set(LCD_G2_PORT, LCD_TLI_AF, LCD_G2_PIN);
    gpio_af_set(LCD_G3_PORT, LCD_TLI_AF, LCD_G3_PIN);
    gpio_af_set(LCD_G4_PORT, LCD_TLI_AF, LCD_G4_PIN);
    gpio_af_set(LCD_G5_PORT, LCD_TLI_AF, LCD_G5_PIN);
    gpio_af_set(LCD_G6_PORT, LCD_TLI_AF, LCD_G6_PIN);
    gpio_af_set(LCD_G7_PORT, LCD_TLI_AF, LCD_G7_PIN);

    /* B0 ~ B7 */
    gpio_af_set(LCD_B0_PORT, LCD_TLI_AF, LCD_B0_PIN);
    gpio_af_set(LCD_B1_PORT, LCD_TLI_AF, LCD_B1_PIN);
    gpio_af_set(LCD_B2_PORT, LCD_TLI_AF, LCD_B2_PIN);
    gpio_af_set(LCD_B3_PORT, LCD_TLI_AF, LCD_B3_PIN);
    gpio_af_set(LCD_B4_PORT, LCD_TLI_AF, LCD_B4_PIN);
    gpio_af_set(LCD_B5_PORT, LCD_TLI_AF, LCD_B5_PIN);
    gpio_af_set(LCD_B6_PORT, LCD_TLI_AF, LCD_B6_PIN);
    gpio_af_set(LCD_B7_PORT, LCD_TLI_AF, LCD_B7_PIN);

    /* 3. 配置模式为复用推挽输出 */
    /* 用宏定义拼装，代码更整洁 */
    gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);
    gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);

    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
                  | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_8 | GPIO_PIN_9
                            | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4
                  | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4
                            | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10);

    gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);

    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* 4. 配置 LCD 背光引脚 (推挽输出) */
    gpio_mode_set(LCD_BL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, LCD_BL_PIN);
    gpio_output_options_set(LCD_BL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_BL_PIN);
    gpio_bit_set(LCD_BL_PORT, LCD_BL_PIN); /* 默认点亮背光 */
}