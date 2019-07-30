/*
 * FreeModbus Libary: user callback functions and buffer define in slave mode
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: user_mb_app.c,v 1.60 2013/11/23 11:49:05 Armink $
 */
#include "board.h"
#include "user_mb_app.h"
#include "drv_autoaddr.h"
#include "drv_ms5837.h"
#include "drv_tmp117.h"
#include "drv_flash.h"
#include "sys_conf.h"

#define SOFT_VER          0x0105
#define MODBUS_UART_PORT  1

/*------------------------Slave mode use these variables----------------------*/
//Slave mode:DiscreteInputs variables
//USHORT   usSDiscInStart                               = S_DISCRETE_INPUT_START;
//#if S_DISCRETE_INPUT_NDISCRETES%8
//UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
//#else
//UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8]  ;
//#endif
//Slave mode:Coils variables
USHORT   usSCoilStart                                 = S_COIL_START;
#if S_COIL_NCOILS%8
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1]                ;
#else
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8]                  ;
#endif
//Slave mode:InputRegister variables
//USHORT   usSRegInStart                                = S_REG_INPUT_START;
//USHORT   usSRegInBuf[S_REG_INPUT_NREGS]               ;
//Slave mode:HoldingRegister variables
USHORT   usSRegHoldStart                              = S_REG_HOLDING_START;
USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS]           ;



enum
{
    MBS_REG_CNT = 0,
    MBS_ADDR_IND,
    MBS_STATUS_IND,
    SOFT_VER_IND,
    CALC_K_IND,
    CALC_B_IND,
    TMP117_TEMP_IND,
//    MS5837_HTEMP_IND,
    MS5837_LTEMP_IND,
//    MS5837_HPRES_IND,
    MS5837_LPRES_IND,    
    MAX_IND,
};

void mbs_sts_update(void)// 更新本地变量到协议栈寄存器中
{
    extern conf_un conf_inst;
    update_ms5837();
    usSRegHoldBuf[MBS_STATUS_IND] = conf_inst.sys_bm;
//    usSRegHoldBuf[MS5837_HTEMP_IND] = ms5837_get_temp()>>16;
    usSRegHoldBuf[MS5837_LTEMP_IND] = ms5837_get_temp()&0x0000ffff;
//    usSRegHoldBuf[MS5837_HPRES_IND] = ms5837_get_pres()>>16;
    usSRegHoldBuf[MS5837_LPRES_IND] = ms5837_get_pres()&0x0000ffff;
    usSRegHoldBuf[TMP117_TEMP_IND] = tmp117_get_temp((int16_t)usSRegHoldBuf[CALC_K_IND],(int16_t)usSRegHoldBuf[CALC_B_IND]);
}

void mbs_sts_init(void)
{
    usSRegHoldBuf[MBS_REG_CNT] = MAX_IND;
    usSRegHoldBuf[SOFT_VER_IND] = SOFT_VER;
    usSRegHoldBuf[MBS_ADDR_IND] = drv_autoaddr_get_addr();
    usSRegHoldBuf[CALC_K_IND] = rd_calc_data()>>16;
    usSRegHoldBuf[CALC_B_IND] = rd_calc_data()&0x0000ffff;
}

void mbs_hw_init(void)
{
#if defined(RT_MODBUS_SLAVE_USE_CONTROL_PIN)
    rt_pin_mode(MODBUS_SLAVE_RT_CONTROL_PIN_INDEX, PIN_MODE_OUTPUT);
    rt_pin_write(MODBUS_SLAVE_RT_CONTROL_PIN_INDEX, PIN_LOW);
#endif
}

void modbus_slave_thread_entry(void* parameter)
{
    extern conf_un conf_inst;
		eMBErrorCode    eStatus = MB_ENOERR;
    mbs_hw_init();
		rt_thread_delay(1000);
  
  
    while(1)
    {
        if(drv_autoaddr_get_addr() == 0)
            rt_thread_mdelay(1000);
        else
        {
            eStatus = eMBInit(MB_RTU, drv_autoaddr_get_addr() , MODBUS_UART_PORT, 9600,  MB_PAR_NONE);
            break;
        }
        
    }    
    iflash_init();
    tmp117_init();
    ms5837_init();  
    
    mbs_sts_init();
    
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MB_SLAVE init failed\n");
		}    
    
		eStatus = eMBEnable();			
		if(eStatus != MB_ENOERR)
		{
        conf_inst.bm_field.mbs = 0;
				rt_kprintf("MB_SLAVE enable failed\n");	
		}    
    conf_inst.bm_field.mbs =1;
		while(1)
		{
        
				eStatus = eMBPoll();
				if(eStatus != MB_ENOERR)
				{
						rt_kprintf("MB_SLAVE poll failed\n");	
				}	
				mbs_sts_update();	
				rt_thread_mdelay(50);
		}
}


/**
 * Modbus slave input register callback function.
 *
 * @param pucRegBuffer input register buffer
 * @param usAddress input register address
 * @param usNRegs input register number
 *
 * @return result
 */
//eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
//{
//    eMBErrorCode    eStatus = MB_ENOERR;
//    USHORT          iRegIndex;
//    USHORT *        pusRegInputBuf;
//    USHORT          REG_INPUT_START;
//    USHORT          REG_INPUT_NREGS;
//    USHORT          usRegInStart;

//    pusRegInputBuf = usSRegInBuf;
//    REG_INPUT_START = S_REG_INPUT_START;
//    REG_INPUT_NREGS = S_REG_INPUT_NREGS;
//    usRegInStart = usSRegInStart;

//    /* it already plus one in modbus function method. */
//    usAddress--;

//    if ((usAddress >= REG_INPUT_START)
//            && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
//    {
//        iRegIndex = usAddress - usRegInStart;
//        while (usNRegs > 0)
//        {
//            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] >> 8);
//            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] & 0xFF);
//            iRegIndex++;
//            usNRegs--;
//        }
//    }
//    else
//    {
//        eStatus = MB_ENOREG;
//    }

//    return eStatus;
//}

/**
 * Modbus slave holding register callback function.
 *
 * @param pucRegBuffer holding register buffer
 * @param usAddress holding register address
 * @param usNRegs holding register number
 * @param eMode read or write
 *
 * @return result
 */
eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress,
        USHORT usNRegs, eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          REG_HOLDING_START;
    USHORT          REG_HOLDING_NREGS;
    USHORT          usRegHoldStart;

    pusRegHoldingBuf = usSRegHoldBuf;
    REG_HOLDING_START = S_REG_HOLDING_START;
    REG_HOLDING_NREGS = S_REG_HOLDING_NREGS;
    usRegHoldStart = usSRegHoldStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_HOLDING_START)
            && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode)
        {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0)
            {
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;

        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (usNRegs > 0)
            {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
                if(iRegIndex == (CALC_B_IND+1))
                {
                    wr_calc_data(pusRegHoldingBuf[CALC_K_IND],pusRegHoldingBuf[CALC_B_IND]);
                }
            }
            break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * Modbus slave coils callback function.
 *
 * @param pucRegBuffer coils buffer
 * @param usAddress coils address
 * @param usNCoils coils number
 * @param eMode read or write
 *
 * @return result
 */
//eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress,
//        USHORT usNCoils, eMBRegisterMode eMode)
//{
//    eMBErrorCode    eStatus = MB_ENOERR;
//    USHORT          iRegIndex , iRegBitIndex , iNReg;
//    UCHAR *         pucCoilBuf;
//    USHORT          COIL_START;
//    USHORT          COIL_NCOILS;
//    USHORT          usCoilStart;
//    iNReg =  usNCoils / 8 + 1;

//    pucCoilBuf = ucSCoilBuf;
//    COIL_START = S_COIL_START;
//    COIL_NCOILS = S_COIL_NCOILS;
//    usCoilStart = usSCoilStart;

//    /* it already plus one in modbus function method. */
//    usAddress--;

//    if( ( usAddress >= COIL_START ) &&
//        ( usAddress + usNCoils <= COIL_START + COIL_NCOILS ) )
//    {
//        iRegIndex = (USHORT) (usAddress - usCoilStart) / 8;
//        iRegBitIndex = (USHORT) (usAddress - usCoilStart) % 8;
//        switch ( eMode )
//        {
//        /* read current coil values from the protocol stack. */
//        case MB_REG_READ:
//            while (iNReg > 0)
//            {
//                *pucRegBuffer++ = xMBUtilGetBits(&pucCoilBuf[iRegIndex++],
//                        iRegBitIndex, 8);
//                iNReg--;
//            }
//            pucRegBuffer--;
//            /* last coils */
//            usNCoils = usNCoils % 8;
//            /* filling zero to high bit */
//            *pucRegBuffer = *pucRegBuffer << (8 - usNCoils);
//            *pucRegBuffer = *pucRegBuffer >> (8 - usNCoils);
//            break;

//            /* write current coil values with new values from the protocol stack. */
//        case MB_REG_WRITE:
//            while (iNReg > 1)
//            {
//                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8,
//                        *pucRegBuffer++);
//                iNReg--;
//            }
//            /* last coils */
//            usNCoils = usNCoils % 8;
//            /* xMBUtilSetBits has bug when ucNBits is zero */
//            if (usNCoils != 0)
//            {
//                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
//                        *pucRegBuffer++);
//            }
//            break;
//        }
//    }
//    else
//    {
//        eStatus = MB_ENOREG;
//    }
//    return eStatus;
//}

/**
 * Modbus slave discrete callback function.
 *
 * @param pucRegBuffer discrete buffer
 * @param usAddress discrete address
 * @param usNDiscrete discrete number
 *
 * @return result
 */
//eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
//{
//    eMBErrorCode    eStatus = MB_ENOERR;
//    USHORT          iRegIndex , iRegBitIndex , iNReg;
//    UCHAR *         pucDiscreteInputBuf;
//    USHORT          DISCRETE_INPUT_START;
//    USHORT          DISCRETE_INPUT_NDISCRETES;
//    USHORT          usDiscreteInputStart;
//    iNReg =  usNDiscrete / 8 + 1;

//    pucDiscreteInputBuf = ucSDiscInBuf;
//    DISCRETE_INPUT_START = S_DISCRETE_INPUT_START;
//    DISCRETE_INPUT_NDISCRETES = S_DISCRETE_INPUT_NDISCRETES;
//    usDiscreteInputStart = usSDiscInStart;

//    /* it already plus one in modbus function method. */
//    usAddress--;

//    if ((usAddress >= DISCRETE_INPUT_START)
//            && (usAddress + usNDiscrete    <= DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES))
//    {
//        iRegIndex = (USHORT) (usAddress - usDiscreteInputStart) / 8;
//        iRegBitIndex = (USHORT) (usAddress - usDiscreteInputStart) % 8;

//        while (iNReg > 0)
//        {
//            *pucRegBuffer++ = xMBUtilGetBits(&pucDiscreteInputBuf[iRegIndex++],
//                    iRegBitIndex, 8);
//            iNReg--;
//        }
//        pucRegBuffer--;
//        /* last discrete */
//        usNDiscrete = usNDiscrete % 8;
//        /* filling zero to high bit */
//        *pucRegBuffer = *pucRegBuffer << (8 - usNDiscrete);
//        *pucRegBuffer = *pucRegBuffer >> (8 - usNDiscrete);
//    }
//    else
//    {
//        eStatus = MB_ENOREG;
//    }

//    return eStatus;
//}

static void mbs_info(void)
{
    extern conf_un conf_inst;
    uint8_t i;
    for(i=0;i<S_REG_HOLDING_NREGS;i++)
    {
        rt_kprintf("reg %d: %x\n",i,usSRegHoldBuf[i]);
    }
}

MSH_CMD_EXPORT(mbs_info, modbus slave holding regs)
