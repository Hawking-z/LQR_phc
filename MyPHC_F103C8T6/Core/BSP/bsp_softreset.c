#include "bsp_softreset.h"
#include "main.h"

void bsp_soft_rest(){
    // 关闭所有中断
    __set_FAULTMASK(1); 
    // 复位
    NVIC_SystemReset();
}