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
#include "drv_soft_i2c.h"
//#include <rtdevice.h>


#define DEMO_I2C_BUS      "i2c1"  /* 传感器连接的I2C总线设备名称 */
#define eeprom_addr         0x50    /* 传感器连接的I2C总线设备地址 */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C总线设备句柄 */

/* 写传感器寄存器 */
static rt_err_t demo_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t addr, rt_uint8_t *data)
{
    rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;

    buf[0] = addr; //cmd
    buf[1] = *data;

    msgs.addr = eeprom_addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

/* 读传感器寄存器数据 */
static rt_err_t demo_read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;

    msgs.addr = eeprom_addr;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

void i2c_demo_init(const char *name)
{
    uint8_t buf[3],temp;
  
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(name);
  
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", name);
    }
    else
    {
        temp = 0x51;
        if(RT_EOK == demo_write_reg(i2c_bus,0,&temp))
            rt_kprintf("wr ok\n");
        rt_thread_delay(100);
        if(RT_EOK == demo_read_regs(i2c_bus,2,buf))
            rt_kprintf("rd ok\n");
        rt_kprintf("read back: %x,%x\n", buf[0],buf[1]);
    }
}


rt_err_t i2c_test(void)
{
    struct rt_i2c_bus_device * i2c_device;
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer1[2];
    rt_uint8_t buffer2[2];
    rt_size_t i;
    i2c_device = rt_i2c_bus_device_find(DEMO_I2C_BUS);
    if(i2c_device == RT_NULL)
    {
        rt_kprintf("i2c bus device %s not found!", DEMO_I2C_BUS);
        return -RT_ENOSYS;
    }
    //step 1: read out.
    buffer1[0] = 0;
    msgs[0].addr = eeprom_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    msgs[1].addr  = eeprom_addr;
    msgs[1].flags = RT_I2C_RD; /* Read from slave */
    msgs[1].buf   = buffer2;
    msgs[1].len   = sizeof(buffer2);
    if( rt_i2c_transfer(i2c_device, msgs, 2) != 2 )
    {
        rt_kprintf("read EEPROM fail!");
    }
    rt_kprintf("\nread EEPROM at I2C address: 0x%02X", eeprom_addr);
    rt_kprintf("0x0000: ");
    for(i=0; i<sizeof(buffer2); i++)
    {
        rt_kprintf("%02X ", buffer2[i]);
    }
    rt_kprintf("");
    //step 2: write back.
    for(i=0; i<sizeof(buffer2); i++)
    {
        buffer2[i] = ~buffer2[i];
    }
    msgs[0].addr = eeprom_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    msgs[1].addr  = eeprom_addr;
    msgs[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msgs[1].buf   = buffer2;
    msgs[1].len   = sizeof(buffer2);
    if( rt_i2c_transfer(i2c_device, msgs, 2) != 2 )
    {
        rt_kprintf("write EEPROM fail!");
    }
    rt_thread_delay(rt_tick_from_millisecond(50)); // FRAM no need.
    //step 3: re-read out.
    buffer1[0] = 0;
    msgs[0].addr = eeprom_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    msgs[1].addr  = eeprom_addr;
    msgs[1].flags = RT_I2C_RD; /* Read from slave */
    msgs[1].buf   = buffer2;
    msgs[1].len   = sizeof(buffer2);
    if( rt_i2c_transfer(i2c_device, msgs, 2) != 2 )
    {
        rt_kprintf("re-read EEPROM fail!");
    }
    rt_kprintf("\nre-read EEPROM at I2C address: 0x%02X", eeprom_addr);
    rt_kprintf("0x0000: ");
    for(i=0; i<sizeof(buffer2); i++)
    {
        rt_kprintf("%02X ", buffer2[i]);
    }
    rt_kprintf("");
    return RT_EOK;
}
