/*!
    \file    bsp_trng.c
    \brief   TRNG BSP 实现
    \version 2026-02-12
*/

#include "bsp_trng.h"
#include <stdio.h>

/* 内部就绪检查，带超时和错误状态打印 */
ErrStatus bsp_trng_ready_check(void)
{
    uint32_t timeout = 0;
    FlagStatus trng_flag = RESET;
    ErrStatus reval = SUCCESS;

    /* 等待随机数据有效 */
    do {
        timeout++;
        trng_flag = trng_flag_get(TRNG_FLAG_DRDY);
    } while((RESET == trng_flag) && (0xFFFF > timeout));

    if(RESET == trng_flag) {
        printf("Error: TRNG can't ready \r\n");
        trng_flag = trng_flag_get(TRNG_FLAG_CECS);
        printf("Clock error current status: %d \r\n", trng_flag);
        trng_flag = trng_flag_get(TRNG_FLAG_SECS);
        printf("Seed error current status: %d \r\n", trng_flag);
        reval = ERROR;
    }

    return reval;
}

/* TRNG 初始化，内部最多重试 3 次 */
ErrStatus bsp_trng_init(void)
{
    uint8_t retry = 0;
    ErrStatus reval = ERROR;

    /* TRNG 模块时钟使能 */
    rcu_periph_clock_enable(RCU_TRNG);

    while((ERROR == reval) && (retry < 3)) {
        /* TRNG 寄存器复位并启动 */
        trng_deinit();
        trng_enable();

        /* 检查 TRNG 工作状态 */
        reval = bsp_trng_ready_check();

        if(ERROR == reval) {
            printf("TRNG init fail, retry %d \r\n", retry + 1);
            retry++;
        }
    }

    return reval;
}

/* 获取 32 位真随机数 */
uint32_t bsp_trng_random_get(void)
{
    if(SUCCESS == bsp_trng_ready_check()) {
        return trng_get_true_random_data();
    }
    return 0;
}

/* 获取 [min, max] 范围内的随机数 */
uint8_t bsp_trng_random_range_get(uint8_t min, uint8_t max)
{
    uint32_t rand_val;

    if(min > max) {
        return 0;
    }

    rand_val = bsp_trng_random_get();
    return (uint8_t)(rand_val % (uint32_t)(max - min + 1) + min);
}