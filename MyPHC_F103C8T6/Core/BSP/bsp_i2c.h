#ifndef _BSP_I2C_H
#define _BSP_I2C_H
#include "stdint.h"

#define BSP_I2C_MEMADD_SIZE_8BIT (0x00000001U)
#define BSP_I2C_MEMADD_SIZE_16BIT (0x00000002U)

void BSP_I2C_Init(void);
uint8_t BSP_I2C_Mem_Write(uint8_t i2c_id, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddSize, uint8_t* data, uint16_t size, uint32_t timeout);
uint8_t BSP_I2C_Mem_Read(uint8_t i2c_id, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddSize, uint8_t* data, uint16_t size, uint32_t timeout);

#endif
