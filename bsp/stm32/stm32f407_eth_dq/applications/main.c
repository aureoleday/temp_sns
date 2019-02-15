/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "i2c_demo.h"
#include "drv_autoaddr.h"

#include "thread_entries.h"
/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(G, 9)

int main(void)
{
    int count = 1;
    rt_thread_t init_thread;
  
    init_thread = rt_thread_create("auto_addr",
                                   autoaddr_thread_entry, RT_NULL,
                                   256, 11, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
    
    init_thread = rt_thread_create("led_th",
                                   led_thread_entry, RT_NULL,
                                   256, 25, 5);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);    
  
    init_thread = rt_thread_create("mbm_slave",
                                   modbus_slave_thread_entry, RT_NULL,
                                   1024, 15, 5);
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
