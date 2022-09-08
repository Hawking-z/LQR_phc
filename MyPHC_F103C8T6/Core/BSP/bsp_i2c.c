#include "bsp_i2c.h"
#include "i2c.h"

#define DEVICE_I2C_CNT 2

typedef struct BSP_I2cTypeDef_t {
    I2C_HandleTypeDef* device;
} BSP_I2cTypeDef;

BSP_I2cTypeDef i2c_devices[DEVICE_I2C_CNT];

void BSP_I2C_Init() {
    i2c_devices[0].device = &hi2c1;
    i2c_devices[1].device = &hi2c2;
}

uint8_t BSP_I2C_Mem_Write(uint8_t i2c_id, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddSize, uint8_t* data, uint16_t size, uint32_t timeout) {
    return HAL_I2C_Mem_Write(i2c_devices[i2c_id].device, DevAddr, MemAddr, MemAddSize, data, size, timeout);
}
uint8_t BSP_I2C_Mem_Read(uint8_t i2c_id, uint16_t DevAddr, uint16_t MemAddr, uint16_t MemAddSize, uint8_t* data, uint16_t size, uint32_t timeout) {
    return HAL_I2C_Mem_Read(i2c_devices[i2c_id].device, DevAddr, MemAddr, MemAddSize, data, size, timeout);
}
