#include "bsp_ipa.h"

void bsp_ipa_init_config(uint32_t src_addr, uint32_t dst_addr, uint16_t width, uint16_t height)
{
    ipa_destination_parameter_struct  ipa_destination_init_struct;
    ipa_foreground_parameter_struct   ipa_fg_init_struct;

    rcu_periph_clock_enable(RCU_IPA);
    ipa_deinit();
    
    ipa_pixel_format_convert_mode_set(IPA_FGTODE);
    
    /* 目标配置 */
    ipa_destination_init_struct.destination_pf = IPA_DPF_RGB565;
    ipa_destination_init_struct.destination_memaddr = dst_addr;
    ipa_destination_init_struct.destination_pregreen = 0;
    ipa_destination_init_struct.destination_preblue = 0;
    ipa_destination_init_struct.destination_prered = 0;
    ipa_destination_init_struct.destination_prealpha = 0;
    ipa_destination_init_struct.destination_lineoff = 0;
    ipa_destination_init_struct.image_height = height;
    ipa_destination_init_struct.image_width = width;
    ipa_destination_init(&ipa_destination_init_struct);

    /* 前景配置 */
    ipa_fg_init_struct.foreground_memaddr = src_addr;
    ipa_fg_init_struct.foreground_pf = FOREGROUND_PPF_RGB565;
    ipa_fg_init_struct.foreground_alpha_algorithm = IPA_FG_ALPHA_MODE_0;
    ipa_fg_init_struct.foreground_prealpha = 0x0;
    ipa_fg_init_struct.foreground_lineoff = 0x0;
    ipa_fg_init_struct.foreground_preblue = 0x0;
    ipa_fg_init_struct.foreground_pregreen = 0x0;
    ipa_fg_init_struct.foreground_prered = 0x0;
    ipa_foreground_init(&ipa_fg_init_struct);
}

void bsp_ipa_start(void)
{
    ipa_transfer_enable();
    while(RESET == ipa_interrupt_flag_get(IPA_INT_FLAG_FTF));
}