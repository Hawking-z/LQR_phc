#ifndef _DATATYPES_H
#define _DATATYPES_H
#ifdef __cplusplus
extern "C" {
#endif
#include "crc16.h"
#include "stdint.h"
#pragma pack(1)
typedef struct _general_data {
    uint8_t len;
    uint8_t* data;
    uint16_t crc16;
} general_data;

#pragma pack()
uint8_t CheckVaild(uint8_t* buffer, uint32_t len);
void DataToBuffer(general_data* data, uint8_t* buffer);
void BufferToData(uint8_t* buffer, general_data* data);
uint8_t BufferToData_Check(uint8_t* buffer, uint32_t len, general_data* data);
#ifdef __cplusplus
}
#endif
#endif