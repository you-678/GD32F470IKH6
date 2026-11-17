#include "app_lcd.h"

/* 图像头文件 */
#include "image1.h"
#include "image2.h"
#include "image3.h"
#include "image4.h"
#include "image5.h"
#include "image6.h"
#include "image7.h"
#include "image8.h"
#include "image9.h"
#include "image10.h"
#include "image11.h"
#include "image12.h"

/* 混合后的目标缓冲区 */
uint8_t blended_address_buffer[58292];

static const uint32_t image_list[] = {
    (uint32_t)&gImage_image1,  (uint32_t)&gImage_image2,
    (uint32_t)&gImage_image3,  (uint32_t)&gImage_image4,
    (uint32_t)&gImage_image5,  (uint32_t)&gImage_image6,
    (uint32_t)&gImage_image7,  (uint32_t)&gImage_image8,
    (uint32_t)&gImage_image9,  (uint32_t)&gImage_image10,
    (uint32_t)&gImage_image11, (uint32_t)&gImage_image12
};
#define IMAGE_COUNT (sizeof(image_list) / sizeof(image_list[0]))

void app_lcd_init(void)
{
    /* 初始化 TLI 硬件 */
    bsp_tli_init();
    
    /* 开启图层0、图层1，请求重装载，并开启 TLI */
    tli_layer_enable(LAYER0);
    tli_layer_enable(LAYER1);
    tli_reload_config(TLI_FRAME_BLANK_RELOAD_EN);
    tli_enable();

    /* 配置混合层（Layer1）的显示地址 */
    bsp_tli_blend_layer1_init((uint32_t)blended_address_buffer, 247, 118);
    tli_reload_config(TLI_REQUEST_RELOAD_EN);
}

void app_lcd_task(void)
{
    static uint8_t index = 0;

    /* 使用 IPA 将图片处理并搬运到混合缓冲区 */
    bsp_ipa_init_config(image_list[index], (uint32_t)blended_address_buffer, 247, 118);
    bsp_ipa_start();

    /* 停顿 50ms */
    delay_1ms(50);

    /* 切换到下一张图片 */
    index++;
    if(index >= IMAGE_COUNT) {
        index = 0;
    }
}