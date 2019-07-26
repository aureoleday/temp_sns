#include <rtthread.h>
#include "drv_soft_i2c.h"
#include "sys_conf.h"
#include "stm32f0xx_hal.h"

#define FLASH_USER_START_ADDR   ((uint32_t)0x0800f000) /* Base @ of Page 0, 1 Kbytes */
#define FLASH_USER_END_ADDR     FLASH_USER_START_ADDR + FLASH_PAGE_SIZE   /* End @ of user Flash area */

#define PROG_FLAG               ((uint32_t)0x9bdf1bdf)


static FLASH_EraseInitTypeDef EraseInitStruct;

static uint32_t iflash_rd_val(uint16_t offset)
{
    uint32_t buf;
    buf = *(__IO uint32_t *)(FLASH_USER_START_ADDR + (offset<<2)); 
    return buf;
}

static int16_t iflash_wr_val(uint16_t offset, uint32_t wr_data)
{
    uint32_t PAGEError;
    int16_t ret = 0;
    uint16_t timeout = 0;
    HAL_FLASH_Unlock();

  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
    EraseInitStruct.NbPages     = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;
    
    while (timeout<10)    
    {
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
        {
            rt_thread_mdelay(1);
            timeout++;
        }
        else
        {
            rt_kprintf("flash erase ok!\n");
            break;
        }        
    }
    
    if(timeout>=10)
    {
        rt_kprintf("flash erase fail!\n");
        ret = -1;        
    }
    else
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (FLASH_USER_START_ADDR), PROG_FLAG) == HAL_OK)
        {
            rt_kprintf("flash flag programed ok!\n");
        }
          
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (FLASH_USER_START_ADDR + (offset<<2)), wr_data) == HAL_OK)
        {
            rt_kprintf("flash program ok!\n");
        }
        else
        {
            rt_kprintf("flash program fail!\n");
            ret = -1;
        }
    }
    HAL_FLASH_Lock();
    return ret;
}

int16_t get_program_flag(void)
{
    if(iflash_rd_val(0) == PROG_FLAG)
        return 0;
    else
        return -1;
}

uint32_t rd_calc_data(void)
{
    uint32_t temp;
    temp = iflash_rd_val(1);
    return temp;
}

int16_t wr_calc_data(int16_t start_dat, int16_t end_dat)
{
    uint32_t buf;
    buf = (((uint32_t)start_dat)<<16)|(((uint32_t)end_dat)&0x0000ffff);
//    rt_kprintf("wr_buf: %x\n",buf);
    if(iflash_wr_val(1,buf) == 0)
        return 0;
    else
        return -1;
}

int16_t iflash_init(void)
{
    extern conf_un conf_inst;
    int16_t ret;
    if(get_program_flag() == 0)
    {
        conf_inst.bm_field.prog = 1;
        rt_kprintf("calc data: %x\n",rd_calc_data());
    }
    else
    {
        conf_inst.bm_field.prog = 0;
        ret = wr_calc_data(0,0);
    }
    return ret;    
}

static void iflash_info(void)
{    
    rt_kprintf("iflash flag: %x\n",iflash_rd_val(0));
    rt_kprintf("calc data: %x\n",iflash_rd_val(1));
}

MSH_CMD_EXPORT(iflash_info, iflash info)



