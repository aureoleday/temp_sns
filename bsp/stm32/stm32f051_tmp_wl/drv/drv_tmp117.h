/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __DRV_TMP117_
#define __DRV_TMP117_

#include <rtthread.h>

void tmp117_init(void);
int16_t tmp117_get_temp(int16_t k, int16_t b);
#endif // __DRV_TMP117_
