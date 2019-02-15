/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __I2C_DEMO__
#define __I2C_DEMO__

#include <rtthread.h>

void i2c_demo_init(const char *name);
rt_err_t i2c_test(void);
#endif
