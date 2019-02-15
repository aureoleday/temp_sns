/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-01-28     scott        first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "thread_entries.h"

/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(A, 15)

int main(void)
{
    int count = 1;
    rt_thread_t init_thread;
    init_thread = rt_thread_create("mbm_slave",
                                   modbus_slave_thread_entry, RT_NULL,
                                   2048, 15, 10);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
    
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
