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
#include "drv_autoaddr.h"
#include "drv_ms5837.h"
#include "drv_tmp117.h"
#include "sys_conf.h"

/* defined the LED0 pin: PA15 */
#define LED0_PIN    GET_PIN(A, 15)
conf_un conf_inst;

static void led_blink(uint16_t period)
{
    if(conf_inst.bm_field.addr_fsm == 0)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(period>>1);    
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(period>>1);
    }
    else if(conf_inst.bm_field.addr_fsm >2)
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(period - (period>>7));    
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(period>>7);
    }
    else
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(period - (period>>2));    
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(period>>2);
    }
}

int main(void)
{
    rt_thread_t init_thread;
    
    init_thread = rt_thread_create("auto_addr",
                                   autoaddr_thread_entry, RT_NULL,
                                   256, 11, 5);
    if (init_thread != RT_NULL)
    {
        rt_thread_startup(init_thread);
        rt_kprintf("addr thread init\n");
    }
    init_thread = rt_thread_create("mbm_slave",
                                   modbus_slave_thread_entry, RT_NULL,
                                   512, 15, 10);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread); 
    
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    
    while (1)
    {
        led_blink(2000); 
    }
    return RT_EOK;
}
