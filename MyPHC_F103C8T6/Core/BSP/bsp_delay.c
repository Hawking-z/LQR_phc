#include "bsp_delay.h"
#include "tim.h"
#include "bsp_time.h"
#define DIY_TIM_HANDLE &htim1

void bsp_delay_us(uint16_t nus)
{
    __HAL_TIM_SET_COUNTER(DIY_TIM_HANDLE, 0);
    __HAL_TIM_ENABLE(DIY_TIM_HANDLE);
    while (__HAL_TIM_GET_COUNTER(DIY_TIM_HANDLE) < nus)
    {
    }
    __HAL_TIM_DISABLE(DIY_TIM_HANDLE);
}


void bsp_delay_ms(uint16_t nms) {
    for (int i = 0; i < nms; ++i) {
        bsp_delay_us(1000);
    }
}