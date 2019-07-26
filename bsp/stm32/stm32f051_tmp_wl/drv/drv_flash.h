/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __DRV_FLASH__
#define __DRV_FLASH__

#include <rtthread.h>
//uint32_t iflash_rd_val(uint16_t offset);
//int16_t iflash_wr_val(uint16_t offset, uint32_t wr_data);
int16_t get_program_flag(void);
uint32_t rd_calc_data(void);
int16_t wr_calc_data(int16_t start_dat, int16_t end_dat);
int16_t iflash_init(void);
#endif // __DRV_FLASH__
