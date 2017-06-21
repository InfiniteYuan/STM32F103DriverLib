# ifndef __CRC_H
# define __CRC_H


#include <stdint.h>


#define CRC16 0x8005

uint16_t CRC16Calculate(const uint8_t *data, uint16_t size);

#endif

