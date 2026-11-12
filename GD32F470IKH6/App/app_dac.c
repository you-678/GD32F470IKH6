#include "app_dac.h"

/*!
    \brief      DAC 阶梯波扫描任务
    \note       假设 main 循环中每次调用包含 10ms 延时，此函数每 100ms 改变一次电压
    \param[in]  无
    \param[out] 无
    \retval     无
*/
void app_dac_scan_task(void) {
    static uint32_t tick = 0;
    static uint16_t dac_val = 0;
    static uint8_t dir = 1; /* 1表示电压上升，0表示电压下降 */

    tick++;
    /* 每 10 次计数 (即 100ms) 改变一次电压 */
    if (tick >= 10) { 
        tick = 0;
        
        if (dir) {
            dac_val += 100; /* 每次增加 100，大约增加 0.08V */
            if (dac_val >= 4095) {
                dac_val = 4095;
                dir = 0; /* 到达最大值，转为下降 */
            }
        } else {
            if (dac_val >= 100) {
                dac_val -= 100;
            } else {
                dac_val = 0;
                dir = 1; /* 到达最小值，转为上升 */
            }
        }
        
        /* 将数值输出到硬件 */
        bsp_dac_set_value(dac_val);
    }
}