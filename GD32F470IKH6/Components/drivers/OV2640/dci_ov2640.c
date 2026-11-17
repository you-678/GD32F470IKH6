#include "dci_ov2640.h"
#include "dci_ov2640_init_table.h"

/* 注意：原厂的 dci_config() 和 ckout0_init() 已删除，
   因为我们已经在 bsp_dci.c 中实现了更规范的引脚配置！ */

/*!
    \brief      DCI camera outsize set
*/
uint8_t ov2640_outsize_set(uint16_t width,uint16_t height)
{
    uint16_t outh;
    uint16_t outw;
    uint8_t temp; 
    if(width%4)return 0xFF;
    if(height%4)return 0xFF;
    outw=width/4;
    outh=height/4; 
    
    bsp_sccb_write(0xFF,0x00);
    bsp_sccb_write(0xE0,0x04);
    bsp_sccb_write(0x5A,outw&0xFF);
    bsp_sccb_write(0x5B,outh&0xFF);
    temp=(outw>>8)&0x03;
    temp|=(outh>>6)&0x04;
    bsp_sccb_write(0x5C,temp);
    bsp_sccb_write(0xE0,0x00);
    return 0;
}

/*!
    \brief      DCI camera initialization (只配置芯片寄存器)
*/
uint8_t dci_ov2640_init(void)
{
    uint8_t i;
    
    /* 假设 bsp_sccb_init 和 bsp_dci_init 已经在 App 层调用过了 */
    /* sccb_config(); -> 已移到 BSP */
    /* dci_config();  -> 已移到 BSP */

    dci_enable();  
    dci_capture_enable();
    dci_capture_disable();
    
    /* 原来这里有个 ckout0_init() 给摄像头提供时钟，建议移到 bsp_dci.c 里 */
    
    delay_1ms(10);
    
    /* OV2640 reset */
    if(bsp_sccb_write(0xFF, 0x01)!=0) return 0xFF;
    if(bsp_sccb_write(0x12, 0x80)!=0) return 0xFF;
    delay_1ms(10);
    
    for(i=0;i<sizeof(ov2640_svga_init_reg_tbl)/2;i++){
        if(0 != bsp_sccb_write(ov2640_svga_init_reg_tbl[i][0],ov2640_svga_init_reg_tbl[i][1])){
            return 0xFF;
        }
    }
    
    delay_1ms(100);
    for(i=0;i<(sizeof(ov2640_rgb565_reg_tbl)/2);i++){
        if(0 != bsp_sccb_write(ov2640_rgb565_reg_tbl[i][0],ov2640_rgb565_reg_tbl[i][1])){
            return 0xFF;
        }
    } 
    delay_1ms(100);
    ov2640_outsize_set(320,240);
    return 0;
}

/*!
    \brief      read the ov2640 manufacturer identifier
*/
uint8_t dci_ov2640_id_read(ov2640_id_struct* ov2640id)
{
    uint8_t temp;
    bsp_sccb_write(0xFF, 0x01);
    if(bsp_sccb_read(OV2640_MIDH,&temp)!=0) return 0xFF;
    ov2640id->manufacturer_id1 = temp;
    if(bsp_sccb_read(OV2640_MIDL,&temp)!=0) return 0xFF;
    ov2640id->manufacturer_id2 = temp;
    if(bsp_sccb_read(OV2640_VER,&temp)!=0) return 0xFF;
    ov2640id->version = temp;
    if(bsp_sccb_read(OV2640_PID,&temp)!=0) return 0xFF;
    ov2640id->pid = temp;

    return 0x00;
}