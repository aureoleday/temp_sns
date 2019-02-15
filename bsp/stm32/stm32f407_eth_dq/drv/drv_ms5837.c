#include <rtthread.h>
#include "drv_soft_i2c.h"
//#include <rtdevice.h>

typedef struct
{
    uint16_t C[7];
    uint32_t D1;
    uint32_t D2;
    int32_t  dT;
    int32_t  TEMP;
    int64_t  OFF;
    int64_t  SENS;
    int32_t  P;
    int32_t  Ti;
    int64_t  OFFi;
    int64_t  SENSi;
    int32_t  TEMP2;
    int32_t  P2;  
}ms5837_st;

ms5837_st ms5837_inst;

#define PRES_I2C_BUS         "i2c1"  /* 传感器连接的I2C总线设备名称 */
#define pres_i2c_addr         0x76    /* 传感器连接的I2C总线设备地址 */
static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C总线设备句柄 */

#define  MS5837_CMD_RESET   0x1E
#define  MS5837_CMD_CD1_8   0x40
#define  MS5837_CMD_CD1_9   0x42
#define  MS5837_CMD_CD1_a   0x44
#define  MS5837_CMD_CD1_b   0x46
#define  MS5837_CMD_CD1_c   0x48
#define  MS5837_CMD_CD1_d   0x4a
#define  MS5837_CMD_CD2_8   0x50
#define  MS5837_CMD_CD2_9   0x52
#define  MS5837_CMD_CD2_a   0x54
#define  MS5837_CMD_CD2_b   0x56
#define  MS5837_CMD_CD2_c   0x58
#define  MS5837_CMD_CD2_d   0x5a
#define  MS5837_CMD_RDADC   0x00
#define  MS5837_CMD_RDPRM   0xa0



static int16_t ms5837_reset(void)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer1[2];
    buffer1[0] = MS5837_CMD_RESET;
    msgs[0].addr = pres_i2c_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    if( rt_i2c_transfer(i2c_bus, msgs, 1) != 1 )
    {
        rt_kprintf("reset ms5837 fail!\n");
        return -1;
    }
    else
        return 0;
}

static int16_t ms5837_rd_prom(uint8_t prom_addr, uint16_t* rd_buf)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer1[2];
    rt_uint8_t buffer2[2];
    buffer1[0] = MS5837_CMD_RDPRM|prom_addr;
    msgs[0].addr = pres_i2c_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    msgs[1].addr  = pres_i2c_addr;
    msgs[1].flags = RT_I2C_RD; /* Read from slave */
    msgs[1].buf   = buffer2;
    msgs[1].len   = sizeof(buffer2);
    if( rt_i2c_transfer(i2c_bus, msgs, 2) != 2 )
    {
        rt_kprintf("read ms5837 prom fail!\n");
        return -1;
    }
    else
    {
        *rd_buf = (buffer2[0]<<8)|buffer2[1];
        return 0;
    }
}

static int16_t ms5837_start_conv(uint8_t conv_type)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer1[2];
    buffer1[0] = MS5837_CMD_CD1_c;
    msgs[0].addr = pres_i2c_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    if( rt_i2c_transfer(i2c_bus, msgs, 1) != 1 )
    {
        rt_kprintf("ms5837 start conv fail!\n");
        return -1;
    }
    else
        return 0;
}

static int16_t ms5837_rd_adc(uint32_t* rd_buf)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t buffer1[2];
    rt_uint8_t buffer2[3];
    buffer1[0] = MS5837_CMD_RDADC;
    msgs[0].addr = pres_i2c_addr;
    msgs[0].flags = RT_I2C_WR; /* Write to slave */
    msgs[0].buf = buffer1; /* eeprom offset. */
    msgs[0].len = 1;
    msgs[1].addr  = pres_i2c_addr;
    msgs[1].flags = RT_I2C_RD; /* Read from slave */
    msgs[1].buf   = buffer2;
    msgs[1].len   = sizeof(buffer2);
    if( rt_i2c_transfer(i2c_bus, msgs, 2) != 2 )
    {
        rt_kprintf("read ms5837 prom fail!\n");
        return -1;
    }
    else
    {
        *rd_buf = (buffer2[0]<<16)|(buffer2[1]<<8)|buffer2[2];
        return 0;
    }
}

static uint32_t ms5837_get_val(uint8_t conv_mode,uint16_t conv_delay)
{
    uint32_t conv_data;
    ms5837_start_conv(conv_mode);
    rt_thread_mdelay(conv_delay);
    ms5837_rd_adc(&conv_data);
    return conv_data;
}

static void update_sensor_raw(void)
{
    ms5837_inst.D1 = ms5837_get_val(MS5837_CMD_CD1_c,100);
    ms5837_inst.D2 = ms5837_get_val(MS5837_CMD_CD2_c,100);    
}

static void calc_1st_ord(void)
{
    ms5837_inst.dT = ms5837_inst.D2 - (ms5837_inst.C[5]>>8);
    if(ms5837_inst.dT<-16776960)
        ms5837_inst.dT = -16776960;
    else if(ms5837_inst.dT>16777215)
        ms5837_inst.dT = 16777215;
    
    ms5837_inst.TEMP = 2000 + ((ms5837_inst.dT*ms5837_inst.C[6])>>23);
    
    ms5837_inst.OFF = (ms5837_inst.C[2]<<16) + ((ms5837_inst.dT*ms5837_inst.C[4])>>7);
    if(ms5837_inst.OFF<-17179344900)
        ms5837_inst.OFF = -17179344900;
    else if(ms5837_inst.OFF>25769410560)
        ms5837_inst.OFF = 25769410560;    
    
    ms5837_inst.SENS = (ms5837_inst.C[1]<<15) + ((ms5837_inst.dT*ms5837_inst.C[3])>>8);
    if(ms5837_inst.SENS<-8589672450)
        ms5837_inst.SENS = -8589672450;
    else if(ms5837_inst.SENS>12884705280)
        ms5837_inst.SENS = 12884705280;    
    
    ms5837_inst.P = (((ms5837_inst.D1*ms5837_inst.SENS)>>21) - ms5837_inst.OFF)>>13;
}


static int16_t ms5337_ds_init(void)
{
    uint8_t i;
    ms5837_reset();
    rt_thread_mdelay(100);
    for(i=0;i<7;i++)
      ms5837_rd_prom(i,&ms5837_inst.C[i]);
    return 0;
}

void ms5837_init(void)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(PRES_I2C_BUS);
  
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", PRES_I2C_BUS);
    }
    else
    {
        rt_kprintf("found ms5837 device\n");
    }
    ms5337_ds_init();
}

void update_ms5837(void)
{
    update_sensor_raw();
    calc_1st_ord();
}

int32_t ms5837_get_temp(void)
{
    return ms5837_inst.TEMP;
}

int32_t ms5837_get_pres(void)
{
    return ms5837_inst.P;
}
