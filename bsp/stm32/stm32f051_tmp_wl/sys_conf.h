/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __SYS_CONF__
#define __SYS_CONF__

#include <rtthread.h>

typedef union 
{
    uint16_t sys_bm;
    struct
    {
      uint16_t mbs:1;      
      uint16_t ms5837:1;   
      uint16_t tmp117:1;
      uint16_t prog:1;
      uint16_t addr_fsm:4;
    }bm_field;
}conf_un;

//typedef struct
//{
//    uint16_t bm_mbs:1;        //0: uinitialized 1: initialized
//    uint16_t bm_ms5837:1;     //0: uinitialized 1: initialized
//    uint16_t bm_tmp117:1;     //0: uinitialized 1: initialized
//    uint16_t bm_addr_fsm:4;   //0: uinitialized 1: initialized
//}conf_st;

#endif //__SYS_CONF__
