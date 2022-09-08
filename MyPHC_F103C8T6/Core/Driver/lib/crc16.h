#ifndef _CRC16_CALC_H
#define _CRC16_CALC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

void CRC16_Init();
uint16_t CRC16_Modbus_calc(uint8_t* data, uint32_t num_bytes);

#ifdef __cplusplus
}
#endif

#endif