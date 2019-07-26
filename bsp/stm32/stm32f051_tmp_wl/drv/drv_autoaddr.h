/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __DRV_ADDR__
#define __DRV_ADDR__

#include <rtthread.h>

void autoaddr_thread_entry(void* parameter);
uint8_t drv_autoaddr_get_addr(void);
uint8_t drv_autoaddr_get_fsm(void);
#endif //__DRV_ADDR__
