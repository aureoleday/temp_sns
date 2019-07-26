/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __DRV_MS5837__
#define __DRV_MS5837__

#include <rtthread.h>

void ms5837_init(void);
void update_ms5837(void);
int32_t ms5837_get_temp(void);
int32_t ms5837_get_pres(void);
#endif // __DRV_MS5837__
