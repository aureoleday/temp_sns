#include <rtthread.h>
#include "drv_soft_i2c.h"

#define TMP_I2C_BUS         "i2c2"  /* 传感器连接的I2C总线设备名称 */
#define tmp_i2c_addr         0x48    /* 传感器连接的I2C总线设备地址 */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C总线设备句柄 */

enum
{
    TMP117_TEMP = 0,
    TMP117_CONF,
    TMP117_HILIM,
    TMP117_LOLIM,
    TMP117_EEPLK,
    TMP117_EEPROM1,
    TMP117_EEPROM2,
    TMP117_TEMP_OFF,
    TMP117_EEPROM3,
    TMP117_DEV_ID = 0xf
};


static int16_t tmp117_wr_reg(uint8_t addr, uint16_t cmd)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer0[3];
    buffer0[0] = addr;
    buffer0[1] = (cmd>>8)&0x0ff;
    buffer0[2] = cmd&0x0ff;
    msgs[0].addr = tmp_i2c_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer0; /* eeprom offset. */
    msgs[0].len = 1;
    if( rt_i2c_transfer(i2c_bus, msgs, 1) != 1 )
    {
        rt_kprintf("write tmp117 fail!\n");
        return -1;
    }
    else
        return 0;
}

static int16_t tmp117_rd_reg(uint8_t addr, uint16_t* rd_buf)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer0[2];
    rt_uint8_t buffer1[2];
    buffer0[0] = addr;
    msgs[0].addr = tmp_i2c_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer0; /* eeprom offset. */
    msgs[0].len = 1;
    msgs[1].addr  = tmp_i2c_addr;
    msgs[1].flags = RT_I2C_RD; /* Read from slave */
    msgs[1].buf   = buffer1;
    msgs[1].len   = sizeof(buffer1);
    if( rt_i2c_transfer(i2c_bus, msgs, 2) != 2 )
    {
        rt_kprintf("read tmp117 fail!\n");
        return -1;
    }
    else
    {
        *rd_buf = (buffer0[0]<<8)|buffer1[1];
        return 0;
    }
}

static void tmp117_reset(void)
{
    tmp117_wr_reg(TMP117_CONF,0x2);
}

static int16_t tmp117_ds_init(void)
{
    tmp117_reset();
    return 0;
}

void tmp117_init(void)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(TMP_I2C_BUS);
  
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", TMP_I2C_BUS);
    }
    else
    {
        rt_kprintf("found ms5837 device\n");
    }
    tmp117_ds_init();
}

int16_t tmp117_get_temp(void)
{
    uint16_t temp;
    tmp117_rd_reg(TMP117_TEMP, &temp);
    return (int16_t)temp;
}

