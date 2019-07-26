#include <rtthread.h>

void bit_op_set(uint16_t *data, uint16_t offset, uint8_t option) 
{		
		if(option)
		{
				*data |= (0x0001 << offset);
		}
		else
		{
				*data &= (~(0x0001 << offset));
		}
}

int16_t bit_op_get(const uint16_t data, uint16_t offset) 
{
    uint8_t ret;
    if(data&(0x0001 << offset))
        ret = 1;
    else
        ret = 0;
    return ret;
}
