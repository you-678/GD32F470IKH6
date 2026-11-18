/*!
    \file    bsp_trng.h
    \brief   TRNG BSP 接口
    \version 2026-02-12
*/

#ifndef BSP_TRNG_H
#define BSP_TRNG_H

#include "gd32f4xx.h"

/* TRNG 初始化（内部带重试），成功返回 SUCCESS，失败返回 ERROR */
ErrStatus bsp_trng_init(void);

/* 检查 TRNG 是否就绪，成功返回 SUCCESS，超时/错误返回 ERROR */
ErrStatus bsp_trng_ready_check(void);

/* 获取一个 32 位真随机数，失败返回 0 */
uint32_t bsp_trng_random_get(void);

/* 获取 [min, max] 范围内的随机数，失败返回 0 */
uint8_t bsp_trng_random_range_get(uint8_t min, uint8_t max);

#endif /* BSP_TRNG_H */