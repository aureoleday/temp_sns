#ifndef __BIT_OP_H
#define	__BIT_OP_H

#include <rtthread.h>
void bit_op_set(uint16_t *data, uint16_t offset, uint8_t option);
int16_t bit_op_get(const uint16_t data, uint16_t offset);

#endif	//__BIT_OP_H

